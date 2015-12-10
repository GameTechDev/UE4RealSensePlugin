#include "RealSensePluginPrivatePCH.h"
#include "RealSenseImpl.h"

// Creates handles to the RealSense Session and SenseManager and iterates over 
// all video capture devices to find a RealSense camera.
//
// Creates three RealSenseDataFrames (background, mid, and foreground) to 
// share RealSense data between the camera processing thread and the main thread.
RealSenseImpl::RealSenseImpl()
{
	session = std::unique_ptr<PXCSession, RealSenseDeleter>(PXCSession::CreateInstance());
	if (session == nullptr)
		RS_LOG(Error, "Failed to create PXCSession")
	else
		RS_LOG(Log, "Created PXCSession")

	senseManager = std::unique_ptr<PXCSenseManager, RealSenseDeleter>(session->CreateSenseManager());
	if (senseManager == nullptr)
		RS_LOG(Error, "Failed to create PXCSenseManager")
	else
		RS_LOG(Log, "Created PXCSenseManager")

	capture = std::unique_ptr<PXCCapture, RealSenseDeleter>(nullptr);
	device = std::unique_ptr<PXCCapture::Device, RealSenseDeleter>(nullptr);

	deviceInfo = {};

	// Loop through video capture devices to find a RealSense Camera
	PXCSession::ImplDesc desc1 = {};
	desc1.group = PXCSession::IMPL_GROUP_SENSOR;
	desc1.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;
	for (int m = 0; ; m++) {
		if (device)
			break;

		PXCSession::ImplDesc desc2 = {};
		if (session->QueryImpl(&desc1, m, &desc2) != PXC_STATUS_NO_ERROR) 
			break;

		PXCCapture* tmp;
		if (session->CreateImpl<PXCCapture>(&desc2, &tmp) != PXC_STATUS_NO_ERROR) 
			continue;
		capture.reset(tmp);

		for (int j = 0; ; j++) {
			if (capture->QueryDeviceInfo(j, &deviceInfo) != PXC_STATUS_NO_ERROR) 
				break;

			if ((deviceInfo.model == PXCCapture::DeviceModel::DEVICE_MODEL_F200) ||
				(deviceInfo.model == PXCCapture::DeviceModel::DEVICE_MODEL_R200) ||
				(deviceInfo.model == PXCCapture::DeviceModel::DEVICE_MODEL_R200_ENHANCED) ||
				(deviceInfo.model == PXCCapture::DeviceModel::DEVICE_MODEL_SR300)) {
				device = std::unique_ptr<PXCCapture::Device, RealSenseDeleter>(capture->CreateDevice(j));
			}
		}
	}

	p3DScan = std::unique_ptr<PXC3DScan, RealSenseDeleter>(nullptr);

	RealSenseFeatureSet = 0;
	colorStreamingEnabled.store(false);
	depthStreamingEnabled.store(false);
	scan3DEnabled.store(false);

	cameraThreadRunning.store(false);

	fgFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());
	midFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());
	bgFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());

	colorResolution = {};
	depthResolution = {};

	if (device != nullptr) {
		PXCPointF32 cfov = device->QueryColorFieldOfView();
		PXCPointF32 dfov = device->QueryDepthFieldOfView();
		colorHorizontalFOV = cfov.x;
		colorVerticalFOV = cfov.y;
		depthHorizontalFOV = dfov.x;
		depthVerticalFOV = dfov.y;
	} 
	else {
		colorHorizontalFOV = 0.0f;
		colorVerticalFOV = 0.0f;
		depthHorizontalFOV = 0.0f;
		depthVerticalFOV = 0.0f;
	}

	scan3DResolution = {};
	scan3DFileFormat = PXC3DScan::FileFormat::OBJ;

	scanStarted.store(false);
	scanStopped.store(false);
	reconstructEnabled.store(false);
	scanCompleted.store(false);
	scan3DImageSizeChanged.store(false);
}

// Terminate the camera thread and release the Core SDK handles.
// SDK Module handles are handled internally and should not be released manually.
RealSenseImpl::~RealSenseImpl() 
{
	cameraThreadRunning.load();
	if (cameraThreadRunning) {
		cameraThreadRunning.store(false);
		cameraThread.join();
	}

//	if (device != nullptr)
//		device->Release();

//	capture->Release();
//	senseManager->Release();
//	session->Release();
}

// Camera Processing Thread
// Initialize the RealSense SenseManager and initiate camera processing loop:
// Step 1: Acquire new camera frame
// Step 2: Load shared settings
// Step 3: Perform Core SDK and middleware processing and store results
//         in background RealSenseDataFrame
// Step 4: Swap the background and mid RealSenseDataFrames
void RealSenseImpl::CameraThread()
{
	uint64 currentFrame = 0;

	fgFrame->number = 0;
	midFrame->number = 0;
	bgFrame->number = 0;

	status = senseManager->Init();
	RS_LOG_STATUS(status, "SenseManager Initialized")
	if (status < PXC_STATUS_NO_ERROR)
		return;

	while (cameraThreadRunning.load() == true) {
		// Acquires new camera frame
		status = senseManager->AcquireFrame(true);
		if (status != PXC_STATUS_NO_ERROR)
			continue;

		bgFrame->number = ++currentFrame;

		// Loads shared settings
		colorStreamingEnabled.load();
		depthStreamingEnabled.load();
		scan3DEnabled.load();
		scanStarted.load();
		scanStopped.load();
		reconstructEnabled.load();

		PXCCapture::Sample* sample = senseManager->QuerySample();

		// Performs Core SDK and middleware processing and store results 
		// in background RealSenseDataFrame
		if (colorStreamingEnabled && (sample->color != nullptr)) {
			CopyColorImageToBuffer(sample->color, bgFrame->colorImage, colorResolution.width, colorResolution.height);
		}

		if (depthStreamingEnabled && (sample->depth != nullptr)) {
			CopyDepthImageToBuffer(sample->depth, bgFrame->depthImage, depthResolution.width, depthResolution.height);
		}

		if (scan3DEnabled) {
			if (scanStarted) {
				PXC3DScan::Configuration config = p3DScan->QueryConfiguration();
				config.startScan = true;
				p3DScan->SetConfiguration(config);
				scanStarted.store(false);
			}

			if (scanStopped) {
				PXC3DScan::Configuration config = p3DScan->QueryConfiguration();
				config.startScan = false;
				p3DScan->SetConfiguration(config);
				scanStopped.store(false);
			}

			PXCImage* scanImage = p3DScan->AcquirePreviewImage();
			if (scanImage != nullptr) {
				UpdateScan3DImageSize(scanImage->QueryInfo());
				CopyColorImageToBuffer(scanImage, bgFrame->scanImage, scan3DResolution.width, scan3DResolution.height);
				scanImage->Release();
			}
			
			if (reconstructEnabled) {
				status = p3DScan->Reconstruct(scan3DFileFormat, scan3DFilename.GetCharArray().GetData());
				reconstructEnabled.store(false);
				scanCompleted.store(true);
			}
		}
		
		senseManager->ReleaseFrame();

		// Swaps background and mid RealSenseDataFrames
		std::unique_lock<std::mutex> lockIntermediate(midFrameMutex);
		bgFrame.swap(midFrame);
	}
}

// If it is not already running, starts a new camera processing thread
void RealSenseImpl::StartCamera() 
{
	if (cameraThreadRunning.load() == false) {
		cameraThreadRunning.store(true);
		cameraThread = std::thread([this]() { CameraThread(); });
	}
}

// If there is a camera processing thread running, this function terminates it. 
// Then it resets the SenseManager pipeline (by closing it and re-enabling the 
// previously specified feature set).
void RealSenseImpl::StopCamera() 
{
	if (cameraThreadRunning.load() == true) {
		cameraThreadRunning.store(false);
		cameraThread.join();
	}
	senseManager->Close();
	EnableRealSenseFeatures(RealSenseFeatureSet);
}

// Swaps the mid and foreground RealSenseDataFrames.
void RealSenseImpl::SwapFrames()
{
	std::unique_lock<std::mutex> lock(midFrameMutex);
	if (fgFrame->number < midFrame->number)
		fgFrame.swap(midFrame);
}

// Enables the specified Core SDK and middleware modules and creates handles
// to the related SDK objects.
void RealSenseImpl::EnableRealSenseFeatures(uint32 featureSet) 
{
	if (session == nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to create Session"));
	else if (senseManager == nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to create SenseManager"));
	else if (device == nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to create Device"));

	RealSenseFeatureSet = featureSet;
	if (featureSet & RealSenseFeature::CAMERA_STREAMING) {
		colorStreamingEnabled.store(true);
		depthStreamingEnabled.store(true);
	}
	if (featureSet & RealSenseFeature::SCAN_3D) {
		senseManager->Enable3DScan();
		p3DScan = std::unique_ptr<PXC3DScan, RealSenseDeleter>(senseManager->Query3DScan());
		scan3DEnabled.store(true);
	}
}

// Returns the connceted device's model as a Blueprintable enum value.
const ECameraModel RealSenseImpl::GetCameraModel() const
{
	switch (deviceInfo.model) {
	case PXCCapture::DeviceModel::DEVICE_MODEL_F200:
		return ECameraModel::F200;
	case PXCCapture::DeviceModel::DEVICE_MODEL_R200:
	case PXCCapture::DeviceModel::DEVICE_MODEL_R200_ENHANCED:
		return ECameraModel::R200;
	case PXCCapture::DeviceModel::DEVICE_MODEL_SR300:
		return ECameraModel::SR300;
	default:
		return ECameraModel::Other;
	}
}

// Returns the connected camera's firmware version as a human-readable string.
const FString RealSenseImpl::GetCameraFirmware() const
{
	return FString::Printf(TEXT("%d.%d.%d.%d"), deviceInfo.firmware[0], deviceInfo.firmware[1], deviceInfo.firmware[2], deviceInfo.firmware[3]);
}

// Enables the color camera stream of the SenseManager using the specified resolution
// and resizes the colorImage buffer of the RealSenseDataFrames to match.
void RealSenseImpl::SetColorCameraResolution(EColorResolution resolution) 
{
	colorResolution = GetEColorResolutionValue(resolution);
	status = senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_COLOR, colorResolution.width, colorResolution.height, colorResolution.fps);
	RS_LOG_STATUS(status, "Enabled Color Stream: %d x %d x %f", colorResolution.width, colorResolution.height, colorResolution.fps)

	if (status == PXC_STATUS_NO_ERROR) {
		int bytesPerPixel = 4;
		bgFrame->colorImage.SetNumZeroed(colorResolution.width * colorResolution.height * bytesPerPixel);
		midFrame->colorImage.SetNumZeroed(colorResolution.width * colorResolution.height * bytesPerPixel);
		fgFrame->colorImage.SetNumZeroed(colorResolution.width * colorResolution.height * bytesPerPixel);
	}
}

// Enables the depth camera stream of the SenseManager using the specified resolution
// and resizes the depthImage buffer of the RealSenseDataFrames to match.
void RealSenseImpl::SetDepthCameraResolution(EDepthResolution resolution)
{
	depthResolution = GetEDepthResolutionValue(resolution);
	status = senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_DEPTH, depthResolution.width, depthResolution.height, depthResolution.fps);
	RS_LOG_STATUS(status, "Enabled Depth Stream: %d x %d x %f", depthResolution.width, depthResolution.height, depthResolution.fps)

	if (status == PXC_STATUS_NO_ERROR) {
		bgFrame->depthImage.SetNumZeroed(depthResolution.width * depthResolution.height);
		midFrame->depthImage.SetNumZeroed(depthResolution.width * depthResolution.height);
		fgFrame->depthImage.SetNumZeroed(depthResolution.width * depthResolution.height);
	}
}

// Creates a StreamProfile for the specified color and depth resolutions and
// uses the RSSDK function IsStreamProfileSetValid to test if the two
// camera resolutions are supported together as a set.
bool RealSenseImpl::IsStreamSetValid(EColorResolution ColorResolution, EDepthResolution DepthResolution)
{
	FStreamResolution CRes = GetEColorResolutionValue(ColorResolution);
	FStreamResolution DRes = GetEDepthResolutionValue(DepthResolution);

	PXCCapture::Device::StreamProfileSet profiles = {};

	PXCImage::ImageInfo colorInfo;
	colorInfo.width = CRes.width;
	colorInfo.height = CRes.height;
	colorInfo.format = ERealSensePixelFormatToPXCPixelFormat(CRes.format);
	colorInfo.reserved = 0;

	profiles.color.imageInfo = colorInfo;
	profiles.color.frameRate = { CRes.fps, CRes.fps };
	profiles.color.options = PXCCapture::Device::StreamOption::STREAM_OPTION_ANY;

	PXCImage::ImageInfo depthInfo;
	depthInfo.width = DRes.width;
	depthInfo.height = DRes.height;
	depthInfo.format = ERealSensePixelFormatToPXCPixelFormat(DRes.format);
	depthInfo.reserved = 0;

	profiles.depth.imageInfo = depthInfo;
	profiles.depth.frameRate = { DRes.fps, DRes.fps };
	profiles.depth.options = PXCCapture::Device::StreamOption::STREAM_OPTION_ANY;

	return (device->IsStreamProfileSetValid(&profiles) != 0);
}

// Creates a new configuration for the 3D Scanning module, specifying the
// scanning mode, solidify, and texture options, and initializing the 
// startScan flag to false to postpone the start of scanning.
bool RealSenseImpl::ConfigureScanning(EScan3DMode scanningMode, bool solidify, bool texture) 
{
	PXC3DScan::Configuration config = {};

	config.mode = ERealSenseScanModeToPXCScanMode(scanningMode);

	config.options = PXC3DScan::ReconstructionOption::NONE;
	if (solidify)
		config.options = config.options | PXC3DScan::ReconstructionOption::SOLIDIFICATION;
	if (texture)
		config.options = config.options | PXC3DScan::ReconstructionOption::TEXTURE;

	config.startScan = false;

	scan3DEnabled.store(true);

	status = p3DScan->SetConfiguration(config);
	RS_LOG_STATUS(status, "Scan3D Configure Scanning")
	return (status == PXC_STATUS_NO_ERROR);
}

// Manually sets the 3D volume in which the 3D scanning module will collect
// data and the voxel resolution to use while scanning.
void RealSenseImpl::SetScanningVolume(FVector boundingBox, int32 resolution)
{
	PXC3DScan::Area area;
	area.shape.width = boundingBox.X;
	area.shape.height = boundingBox.Y;
	area.shape.depth = boundingBox.Z;
	area.resolution = resolution;

	status = p3DScan->SetArea(area);
	RS_LOG_STATUS(status, "Scanning Area = %f x %f x %f, Voxel Size = %d", boundingBox.X, boundingBox.Y, boundingBox.Z, resolution)
}

// Sets the scanStarted flag to true. On the next iteration of the camera
// processing loop, it will load this flag and tell the 3D Scanning configuration
// to begin scanning.
void RealSenseImpl::StartScanning() 
{
	scanStarted.store(true);
	scanCompleted.store(false);
}

// Sets the scanStopped flag to true. On the next iteration of the camera
// processing loop, it will load this flag and tell the 3D Scanning configuration
// to stop scanning.
void RealSenseImpl::StopScanning()
{
	scanStopped.store(true);
}

// Manually resets the scanning process by querying for the current 3D Scanning
// configuration and setting the configuration with the result.
void RealSenseImpl::ResetScanning()
{
	scan3DEnabled.store(false);
	p3DScan->SetConfiguration(p3DScan->QueryConfiguration());
	scan3DEnabled.store(true);
}

// Stores the file format and filename to use for saving the scan and sets the
// reconstructEnabled flag to true. On the next iteration of the camera processing
// loop, it will load this flag and reconstruct the scanned data as a mesh file.
void RealSenseImpl::SaveScan(EScan3DFileFormat saveFileFormat, const FString& filename) 
{
	scan3DFileFormat = static_cast<PXC3DScan::FileFormat> (saveFileFormat);
	scan3DFilename = filename;
	reconstructEnabled.store(true);
}

// Loads the specified mesh file (.obj) into memory and parses it to extract the 
// vertices, triangles, and vertex colors.
//
// This function then finds the average vertex position (the mesh center) and 
// moves all vertices to be centered about that point.
void RealSenseImpl::LoadScan(const FString& filename, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FColor>& Colors)
{
	Vertices.Empty();
	Triangles.Empty();
	Colors.Empty();

	std::ifstream file;
	file.open(filename.GetCharArray().GetData());

	if (!file.is_open())
		return;
	

//	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
//	IFileHandle* FileHandle = PlatformFile.OpenRead(filename.GetCharArray().GetData(), false);

//	if (FileHandle == nullptr)
//		return;

	float x, y, z, r, g, b = 0.0f;
	int v1, v2, v3, n1, n2, n3 = 0;
	std::string line;

	while (std::getline(file, line)) {
		if (line[0] == 'v') {
			if (line[1] == ' ') {
				sscanf_s(line.substr(2).c_str(), "%f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);
				Vertices.Add(ConvertRSVectorToUnreal(FVector(x, y, z)) * 150);
				Colors.Add(FColor((uint8)(r * 255), (uint8)(g * 255), (uint8)(b * 255)));
			}
		}
		else if (line[0] == 'f') {
			sscanf_s(line.substr(1).c_str(), "%d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3);
			// Need to subtract 1 from the vertex indices because .OBJ files start indexing them at at 1, not 0
			Triangles.Add(v1 - 1);
			Triangles.Add(v2 - 1);
			Triangles.Add(v3 - 1);
		}
	}

	FVector MeshCenter = FVector(0.0f, 0.0f, 0.0f);
	for (FVector Vert : Vertices) {
		MeshCenter += Vert;
	}

	MeshCenter /= Vertices.Num();

	for (int i = 0; i < Vertices.Num(); i++) {
		Vertices[i] -= MeshCenter;
	}
}

// The input ImageInfo object contains the wight and height of the preview image
// provided by the 3D Scanning module. The image size can be changed automatically
// by the middleware, so this function checks if the size has changed.
//
// If true, sets the 3D scan resolution to reflect the new size and resizes the
// scanImage buffer of the RealSenseDataFrames to match.
void RealSenseImpl::UpdateScan3DImageSize(PXCImage::ImageInfo info) 
{
	if ((scan3DResolution.width == info.width) && (scan3DResolution.height == info.height)) {
		return;
	}

	scan3DResolution.width = info.width;
	scan3DResolution.height = info.height;

	int bytesPerPixel = 4;
	bgFrame->scanImage.SetNumZeroed(scan3DResolution.width * scan3DResolution.height * bytesPerPixel);
	midFrame->scanImage.SetNumZeroed(scan3DResolution.width * scan3DResolution.height * bytesPerPixel);
	fgFrame->scanImage.SetNumZeroed(scan3DResolution.width * scan3DResolution.height * bytesPerPixel);

	scan3DImageSizeChanged.store(true);
}
