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
	assert(session != nullptr);

	senseManager = std::unique_ptr<PXCSenseManager, RealSenseDeleter>(session->CreateSenseManager());
	assert(senseManager != nullptr);

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
	pFace = std::unique_ptr<PXCFaceModule, RealSenseDeleter>(nullptr);
	pBlob = std::unique_ptr<PXCBlobModule, RealSenseDeleter>(nullptr);

	RealSenseFeatureSet = 0;
	bCameraStreamingEnabled = false;
	bScan3DEnabled = false;
	bFaceEnabled = false;
	bBlobEnabled = false;

	bCameraThreadRunning = false;

	fgFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());
	midFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());
	bgFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());

	colorResolution = {};
	depthResolution = {};

	if (device == nullptr) {
		colorHorizontalFOV = 0.0f;
		colorVerticalFOV = 0.0f;
		depthHorizontalFOV = 0.0f;
		depthVerticalFOV = 0.0f;
	} 
	else {
		PXCPointF32 cfov = device->QueryColorFieldOfView();
		colorHorizontalFOV = cfov.x;
		colorVerticalFOV = cfov.y;

		PXCPointF32 dfov = device->QueryDepthFieldOfView();
		depthHorizontalFOV = dfov.x;
		depthVerticalFOV = dfov.y;
	}

	scan3DResolution = {};
	scan3DFileFormat = PXC3DScan::FileFormat::OBJ;

	bScanStarted = false;
	bScanStopped = false;
	bReconstructEnabled = false;
	bScanCompleted = false;
	bScan3DImageSizeChanged = false;
}

// Terminate the camera thread and release the Core SDK handles.
// SDK Module handles are handled internally and should not be released manually.
RealSenseImpl::~RealSenseImpl() 
{
	if (bCameraThreadRunning) {
		bCameraThreadRunning = false;
		cameraThread.join();
	}
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

	pxcStatus status = senseManager->Init();
	RS_LOG_STATUS(status, "SenseManager Initialized")

	assert(status == PXC_STATUS_NO_ERROR);

	if (bFaceEnabled) {
		faceData = pFace->CreateOutput();
	}

	if (bBlobEnabled) {
		blobData = pBlob->CreateOutput();
	}

	while (bCameraThreadRunning == true) {
		// Acquires new camera frame
		status = senseManager->AcquireFrame(true);
		assert(status == PXC_STATUS_NO_ERROR);

		bgFrame->number = ++currentFrame;

		// Performs Core SDK and middleware processing and store results 
		// in background RealSenseDataFrame
		if (bCameraStreamingEnabled) {
			PXCCapture::Sample* sample = senseManager->QuerySample();

			CopyColorImageToBuffer(sample->color, bgFrame->colorImage, colorResolution.width, colorResolution.height);
			CopyDepthImageToBuffer(sample->depth, bgFrame->depthImage, depthResolution.width, depthResolution.height);
		}

		if (bScan3DEnabled) {
			if (bScanStarted) {
				PXC3DScan::Configuration config = p3DScan->QueryConfiguration();
				config.startScan = true;
				p3DScan->SetConfiguration(config);
				bScanStarted = false;
			}

			if (bScanStopped) {
				PXC3DScan::Configuration config = p3DScan->QueryConfiguration();
				config.startScan = false;
				p3DScan->SetConfiguration(config);
				bScanStopped = false;
			}

			PXCImage* scanImage = p3DScan->AcquirePreviewImage();
			if (scanImage) {
				UpdateScan3DImageSize(scanImage->QueryInfo());
				CopyColorImageToBuffer(scanImage, bgFrame->scanImage, scan3DResolution.width, scan3DResolution.height);
				scanImage->Release();
			}
			
			if (bReconstructEnabled) {
				status = p3DScan->Reconstruct(scan3DFileFormat, scan3DFilename.GetCharArray().GetData());
				bReconstructEnabled = false;
				bScanCompleted = true;
			}
		}

		if (bFaceEnabled) {
			faceData->Update();
			bgFrame->headCount = faceData->QueryNumberOfDetectedFaces();
			if (bgFrame->headCount > 0) {
				PXCFaceData::Face* face = faceData->QueryFaceByIndex(0);
				PXCFaceData::PoseData* poseData = face->QueryPose();

				if (poseData) {
					PXCFaceData::HeadPosition headPosition = {};
					poseData->QueryHeadPosition(&headPosition);
					bgFrame->headPosition = FVector(headPosition.headCenter.x, headPosition.headCenter.y, headPosition.headCenter.z);

					PXCFaceData::PoseEulerAngles headRotation = {};
					poseData->QueryPoseAngles(&headRotation);
					bgFrame->headRotation = FRotator(headRotation.pitch, headRotation.yaw, headRotation.roll);
				}
			}
		}

		if (bBlobEnabled) {
			blobData->Update();
			bgFrame->blobCount = blobData->QueryNumberOfBlobs();
			if (bgFrame->blobCount > 0) {
				PXCBlobData::IBlob *pBlob = {};
				blobData->QueryBlob(0, PXCBlobData::SEGMENTATION_IMAGE_DEPTH, PXCBlobData::ACCESS_ORDER_LARGE_TO_SMALL, pBlob);

				if(pBlob) {
					bgFrame->blobPixels.push_back(pBlob->QueryPixelCount());

					PXCPoint3DF32 center = pBlob->QueryExtremityPoint(PXCBlobData::EXTREMITY_CENTER);
					bgFrame->blobCenter = FVector(center.x, center.y, center.z);

					PXCPoint3DF32 closest = pBlob->QueryExtremityPoint(PXCBlobData::EXTREMITY_CLOSEST);
					bgFrame->blobClosest = FVector(closest.x, closest.y, closest.z);

					PXCPoint3DF32 top = pBlob->QueryExtremityPoint(PXCBlobData::EXTREMITY_TOP_MOST);
					bgFrame->blobTop = FVector(top.x, top.y, top.z);

					PXCPoint3DF32 bottom = pBlob->QueryExtremityPoint(PXCBlobData::EXTREMITY_BOTTOM_MOST);
					bgFrame->blobBottom = FVector(bottom.x, bottom.y, bottom.z);

					PXCPoint3DF32 left = pBlob->QueryExtremityPoint(PXCBlobData::EXTREMITY_LEFT_MOST);
					bgFrame->blobLeft = FVector(left.x, left.y, left.z);

					PXCPoint3DF32 right = pBlob->QueryExtremityPoint(PXCBlobData::EXTREMITY_RIGHT_MOST);
					bgFrame->blobRight = FVector(right.x, right.y, right.z);
				}
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
	if (bCameraThreadRunning == false) {
		EnableMiddleware();
		bCameraThreadRunning = true;
		cameraThread = std::thread([this]() { CameraThread(); });
	}
}

// If there is a camera processing thread running, this function terminates it. 
// Then it resets the SenseManager pipeline (by closing it and re-enabling the 
// previously specified feature set).
void RealSenseImpl::StopCamera() 
{
	if (bCameraThreadRunning) {
		bCameraThreadRunning = false;
		cameraThread.join();
	}
	senseManager->Close();
}

// Swaps the mid and foreground RealSenseDataFrames.
void RealSenseImpl::SwapFrames()
{
	std::unique_lock<std::mutex> lock(midFrameMutex);
	if (fgFrame->number < midFrame->number) {
		fgFrame.swap(midFrame);
	}
}

void RealSenseImpl::EnableMiddleware()
{
	if (bScan3DEnabled) {
		senseManager->Enable3DScan();
		p3DScan = std::unique_ptr<PXC3DScan, RealSenseDeleter>(senseManager->Query3DScan());
	}
	if (bFaceEnabled) {
		senseManager->EnableFace();
		pFace = std::unique_ptr<PXCFaceModule, RealSenseDeleter>(senseManager->QueryFace());
	}
	if (bBlobEnabled) {
		senseManager->EnableBlob();
		pBlob = std::unique_ptr<PXCBlobModule, RealSenseDeleter>(senseManager->QueryBlob());
	}
}

void RealSenseImpl::EnableFeature(RealSenseFeature feature)
{
	switch (feature) {
	case RealSenseFeature::CAMERA_STREAMING:
		bCameraStreamingEnabled = true;
		return;
	case RealSenseFeature::SCAN_3D:
		bScan3DEnabled = true;
		return;
	case RealSenseFeature::HEAD_TRACKING:
		bFaceEnabled = true;
		return;
	case RealSenseFeature::BLOB_TRACKING:
		bBlobEnabled = true;
		return;
	}
}

void RealSenseImpl::DisableFeature(RealSenseFeature feature)
{
	switch (feature) {
	case RealSenseFeature::CAMERA_STREAMING:
		bCameraStreamingEnabled = false;
		return;
	case RealSenseFeature::SCAN_3D:
		bScan3DEnabled = false;
		return;
	case RealSenseFeature::HEAD_TRACKING:
		bFaceEnabled = false;
		return;
	case RealSenseFeature::BLOB_TRACKING:
		bBlobEnabled = true;
		return;
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
	return FString::Printf(TEXT("%d.%d.%d.%d"), deviceInfo.firmware[0], 
												deviceInfo.firmware[1], 
												deviceInfo.firmware[2], 
												deviceInfo.firmware[3]);
}

// Enables the color camera stream of the SenseManager using the specified resolution
// and resizes the colorImage buffer of the RealSenseDataFrames to match.
void RealSenseImpl::SetColorCameraResolution(EColorResolution resolution) 
{
	colorResolution = GetEColorResolutionValue(resolution);

	status = senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_COLOR, 
										colorResolution.width, 
										colorResolution.height, 
										colorResolution.fps);

	assert(status == PXC_STATUS_NO_ERROR);

	const uint8 bytesPerPixel = 4;
	const uint32 colorImageSize = colorResolution.width * colorResolution.height * bytesPerPixel;
	bgFrame->colorImage.SetNumZeroed(colorImageSize);
	midFrame->colorImage.SetNumZeroed(colorImageSize);
	fgFrame->colorImage.SetNumZeroed(colorImageSize);
}

// Enables the depth camera stream of the SenseManager using the specified resolution
// and resizes the depthImage buffer of the RealSenseDataFrames to match.
void RealSenseImpl::SetDepthCameraResolution(EDepthResolution resolution)
{
	depthResolution = GetEDepthResolutionValue(resolution);
	status = senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_DEPTH, 
										depthResolution.width, 
										depthResolution.height, 
										depthResolution.fps);

	assert(status == PXC_STATUS_NO_ERROR);

	if (status == PXC_STATUS_NO_ERROR) {
		const uint32 depthImageSize = depthResolution.width * depthResolution.height;
		bgFrame->depthImage.SetNumZeroed(depthImageSize);
		midFrame->depthImage.SetNumZeroed(depthImageSize);
		fgFrame->depthImage.SetNumZeroed(depthImageSize);
	}
}

// Creates a StreamProfile for the specified color and depth resolutions and
// uses the RSSDK function IsStreamProfileSetValid to test if the two
// camera resolutions are supported together as a set.
bool RealSenseImpl::IsStreamSetValid(EColorResolution ColorResolution, EDepthResolution DepthResolution) const
{
	FStreamResolution CRes = GetEColorResolutionValue(ColorResolution);
	FStreamResolution DRes = GetEDepthResolutionValue(DepthResolution);

	PXCCapture::Device::StreamProfileSet profiles = {};

	PXCImage::ImageInfo colorInfo;
	colorInfo.width = CRes.width;
	colorInfo.height = CRes.height;
	colorInfo.format = GetPXCPixelFormat(CRes.format);
	colorInfo.reserved = 0;

	profiles.color.imageInfo = colorInfo;
	profiles.color.frameRate = { CRes.fps, CRes.fps };
	profiles.color.options = PXCCapture::Device::StreamOption::STREAM_OPTION_ANY;

	PXCImage::ImageInfo depthInfo;
	depthInfo.width = DRes.width;
	depthInfo.height = DRes.height;
	depthInfo.format = GetPXCPixelFormat(DRes.format);
	depthInfo.reserved = 0;

	profiles.depth.imageInfo = depthInfo;
	profiles.depth.frameRate = { DRes.fps, DRes.fps };
	profiles.depth.options = PXCCapture::Device::StreamOption::STREAM_OPTION_ANY;

	return (device->IsStreamProfileSetValid(&profiles) != 0);
}

// Creates a new configuration for the 3D Scanning module, specifying the
// scanning mode, solidify, and texture options, and initializing the 
// startScan flag to false to postpone the start of scanning.
void RealSenseImpl::ConfigureScanning(EScan3DMode scanningMode, bool bSolidify, bool bTexture) 
{
	PXC3DScan::Configuration config = {};

	config.mode = GetPXCScanningMode(scanningMode);

	config.options = PXC3DScan::ReconstructionOption::NONE;
	if (bSolidify) {
		config.options = config.options | PXC3DScan::ReconstructionOption::SOLIDIFICATION;
	}
	if (bTexture) {
		config.options = config.options | PXC3DScan::ReconstructionOption::TEXTURE;
	}

	config.startScan = false;

	status = p3DScan->SetConfiguration(config);
	assert(status == PXC_STATUS_NO_ERROR);
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
	assert(status == PXC_STATUS_NO_ERROR);
}

// Sets the scanStarted flag to true. On the next iteration of the camera
// processing loop, it will load this flag and tell the 3D Scanning configuration
// to begin scanning.
void RealSenseImpl::StartScanning() 
{
	bScanStarted = true;
	bScanCompleted = false;
}

// Sets the scanStopped flag to true. On the next iteration of the camera
// processing loop, it will load this flag and tell the 3D Scanning configuration
// to stop scanning.
void RealSenseImpl::StopScanning()
{
	bScanStopped = true;
}

// Stores the file format and filename to use for saving the scan and sets the
// reconstructEnabled flag to true. On the next iteration of the camera processing
// loop, it will load this flag and reconstruct the scanned data as a mesh file.
void RealSenseImpl::SaveScan(EScan3DFileFormat saveFileFormat, const FString& filename) 
{
	scan3DFileFormat = GetPXCScanFileFormat(saveFileFormat);
	scan3DFilename = filename;
	bReconstructEnabled = true;
}

// The input ImageInfo object contains the wight and height of the preview image
// provided by the 3D Scanning module. The image size can be changed automatically
// by the middleware, so this function checks if the size has changed.
//
// If true, sets the 3D scan resolution to reflect the new size and resizes the
// scanImage buffer of the RealSenseDataFrames to match.
void RealSenseImpl::UpdateScan3DImageSize(PXCImage::ImageInfo info) 
{
	if ((scan3DResolution.width == info.width) && 
		(scan3DResolution.height == info.height)) {
		bScan3DImageSizeChanged = false;
		return;
	}

	scan3DResolution.width = info.width;
	scan3DResolution.height = info.height;

	const uint8 bytesPerPixel = 4;
	const uint32 scanImageSize = scan3DResolution.width * scan3DResolution.height * bytesPerPixel;
	bgFrame->scanImage.SetNumZeroed(scanImageSize);
	midFrame->scanImage.SetNumZeroed(scanImageSize);
	fgFrame->scanImage.SetNumZeroed(scanImageSize);

	bScan3DImageSizeChanged = true;
}
