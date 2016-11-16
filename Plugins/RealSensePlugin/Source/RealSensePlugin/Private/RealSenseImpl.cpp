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
	m_device = std::unique_ptr<PXCCapture::Device, RealSenseDeleter>(nullptr);
	deviceInfo = {};

	// Loop through video capture devices to find a RealSense Camera
	PXCSession::ImplDesc desc1 = {};
	desc1.group = PXCSession::IMPL_GROUP_SENSOR;
	desc1.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;
	for (int m = 0; ; m++) {
		if (m_device)
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
				m_device = std::unique_ptr<PXCCapture::Device, RealSenseDeleter>(capture->CreateDevice(j));
			}
		}
	}

	p3DScan = std::unique_ptr<PXC3DScan, RealSenseDeleter>(nullptr);
	pFace = std::unique_ptr<PXCFaceModule, RealSenseDeleter>(nullptr);
	pHandCursor = std::unique_ptr<PXCHandCursorModule, RealSenseDeleter>(nullptr);

	RealSenseFeatureSet = 0;
	bCameraStreamingEnabled = false;
	bScan3DEnabled = false;
	bFaceEnabled = false;
	bSeg3DEnabled = false;
	bHandCursorEnabled = false;

	bCameraThreadRunning = false;

	fgFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());
	midFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());
	bgFrame = std::unique_ptr<RealSenseDataFrame>(new RealSenseDataFrame());

	colorResolution = {};
	depthResolution = {};

	if (m_device == nullptr) {
		colorHorizontalFOV = 0.0f;
		colorVerticalFOV = 0.0f;
		depthHorizontalFOV = 0.0f;
		depthVerticalFOV = 0.0f;
	} 
	else {
		PXCPointF32 cfov = m_device->QueryColorFieldOfView();
		colorHorizontalFOV = cfov.x;
		colorVerticalFOV = cfov.y;

		PXCPointF32 dfov = m_device->QueryDepthFieldOfView();
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
	while (bCameraThreadRunning == true) {
		EnableMiddleware();

		uint64 currentFrame = 0;

		fgFrame->number = 0;
		midFrame->number = 0;
		bgFrame->number = 0;

		pxcStatus status = senseManager->Init();
		if (status != PXC_STATUS_NO_ERROR) {
			senseManager->Close();
			continue;
		}

		PXCCapture::Device* device = nullptr;
		PXCCapture::Device::MirrorMode MirrorMode = PXCCapture::Device::MIRROR_MODE_DISABLED;
		PXCCapture::Device::MirrorMode NewMirrorMode = PXCCapture::Device::MIRROR_MODE_DISABLED;
		if (bSeg3DEnabled)
		{
			// Get pointer to active device
			device = senseManager->QueryCaptureManager()->QueryDevice();
			if (device)
			{
				// Mirror mode
				NewMirrorMode = PXCCapture::Device::MIRROR_MODE_HORIZONTAL;
				MirrorMode = device->QueryMirrorMode();
				device->SetMirrorMode(NewMirrorMode);
			}
		}

		if (bFaceEnabled) {
			faceData = pFace->CreateOutput();
		}

		PXCCursorData *cursorData = nullptr;
		if (bHandCursorEnabled) {
			cursorData = pHandCursor->CreateOutput();
		}

		while (bCameraThreadRunning == true) {
			// Acquires new camera frame
			status = senseManager->AcquireFrame(true);
			if (status != PXC_STATUS_NO_ERROR) {
				break;
			}

			bgFrame->number = ++currentFrame;

			// Performs Core SDK and middleware processing and store results 
			// in background RealSenseDataFrame

			if (bSeg3DEnabled)
			{
				PXCImage* segmentedImage = p3DSeg->AcquireSegmentedImage();
				if (segmentedImage)
				{
					CopySegmentedImageToBuffer(segmentedImage, bgFrame->colorImage, colorResolution.width, colorResolution.height);
					SAFE_RELEASE(segmentedImage);
				}
			}

			else if (bCameraStreamingEnabled) {
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
						headRotation.pitch = FMath::Clamp<float>(headRotation.pitch, -90.0f, 90.0f);
						headRotation.yaw = FMath::Clamp<float>(headRotation.yaw, -90.0f, 90.0f);
						headRotation.roll = FMath::Clamp<float>(headRotation.roll, -90.0f, 90.0f);
						if (NewMirrorMode == PXCCapture::Device::MIRROR_MODE_HORIZONTAL) {
							headRotation.pitch /= -90.0f;
							headRotation.yaw /= -90.0f;
							headRotation.roll /= -90.0f;
						}
						else {
							headRotation.pitch /= -90.0f;
							headRotation.yaw /= 90.0f;
							headRotation.roll /= -90.0f;
						}
						bgFrame->headRotation = FRotator(headRotation.pitch, headRotation.yaw, headRotation.roll);
					}
				}
			}

			if (bHandCursorEnabled) {
				int BodySideLeftID = 0;
				int BodySideRightID = 0;

				bgFrame->cursorData = FVector::ZeroVector;
				bgFrame->isCursorDataValid = false;
				bgFrame->cursorDataLeft = FVector::ZeroVector;
				bgFrame->isCursorDataLeftValid = false;
				bgFrame->cursorDataRight = FVector::ZeroVector;
				bgFrame->isCursorDataRightValid = false;

				bgFrame->gestureClick = false;
				bgFrame->gestureClockwiseCircle = false;
				bgFrame->gestureCounterClockwiseCircle = false;
				bgFrame->gestureHandClosing = false;
				bgFrame->gestureHandOpening = false;

				bgFrame->bodySideClick = PXCCursorData::BodySideType::BODY_SIDE_UNKNOWN;
				bgFrame->bodySideClockwiseCircle = PXCCursorData::BodySideType::BODY_SIDE_UNKNOWN;
				bgFrame->bodySideCounterClockwiseCircle = PXCCursorData::BodySideType::BODY_SIDE_UNKNOWN;
				bgFrame->bodySideHandClosing = PXCCursorData::BodySideType::BODY_SIDE_UNKNOWN;
				bgFrame->bodySideHandOpening = PXCCursorData::BodySideType::BODY_SIDE_UNKNOWN;

				bgFrame->firedAlertData.Empty();

				cursorData->Update();

				int nCursors = cursorData->QueryNumberOfCursors();
				nCursors = std::min<int>(nCursors, 2);
				for (int i = 0; i < nCursors; ++i) {
					// retrieve the cursor data by order-based index
					PXCCursorData::ICursor *icursor = nullptr;
					PXCCursorData::BodySideType bodySide = PXCCursorData::BodySideType::BODY_SIDE_UNKNOWN;
					status = cursorData->QueryCursorData(PXCCursorData::ACCESS_ORDER_NEAR_TO_FAR, i, icursor);
					if ((status == pxcStatus::PXC_STATUS_NO_ERROR) && icursor) {
						bodySide = icursor->QueryBodySide();
						PXCPoint3DF32 point = icursor->QueryAdaptivePoint();
						point.x = (0.5f - point.x) * 2.0f;
						point.y = (0.5f - point.y) * 2.0f;
						point.z = (0.5f - point.z) * 2.0f;
						FVector vec = FVector(point.x, point.y, point.z);
						if (i == 0) {
							bgFrame->cursorData = vec;
							bgFrame->isCursorDataValid = true;
						}
						if (bodySide == PXCCursorData::BodySideType::BODY_SIDE_LEFT) {
							bgFrame->cursorDataLeft = vec;
							bgFrame->isCursorDataLeftValid = true;
							BodySideLeftID = icursor->QueryUniqueId();
						}
						else if (bodySide == PXCCursorData::BodySideType::BODY_SIDE_RIGHT) {
							bgFrame->cursorDataRight = vec;
							bgFrame->isCursorDataRightValid = true;
							BodySideRightID = icursor->QueryUniqueId();
						}
					}
				}

				// poll for gestures
				PXCCursorData::GestureData gesture;
				if (cursorData->IsGestureFired(PXCCursorData::CURSOR_CLICK, gesture)) {
					bgFrame->gestureClick = true;
					if (gesture.handId == BodySideLeftID) {
						bgFrame->bodySideClick = PXCCursorData::BodySideType::BODY_SIDE_LEFT;
					}
					else if (gesture.handId == BodySideRightID) {
						bgFrame->bodySideClick = PXCCursorData::BodySideType::BODY_SIDE_RIGHT;
					}
				}
				if (cursorData->IsGestureFired(PXCCursorData::CURSOR_CLOCKWISE_CIRCLE, gesture)) {
					bgFrame->gestureClockwiseCircle = true;
					if (gesture.handId == BodySideLeftID) {
						bgFrame->bodySideClockwiseCircle = PXCCursorData::BodySideType::BODY_SIDE_LEFT;
					}
					else if (gesture.handId == BodySideRightID) {
						bgFrame->bodySideClockwiseCircle = PXCCursorData::BodySideType::BODY_SIDE_RIGHT;
					}
				}
				if (cursorData->IsGestureFired(PXCCursorData::CURSOR_COUNTER_CLOCKWISE_CIRCLE, gesture)) {
					bgFrame->gestureCounterClockwiseCircle = true;
					if (gesture.handId == BodySideLeftID) {
						bgFrame->bodySideCounterClockwiseCircle = PXCCursorData::BodySideType::BODY_SIDE_LEFT;
					}
					else if (gesture.handId == BodySideRightID) {
						bgFrame->bodySideCounterClockwiseCircle = PXCCursorData::BodySideType::BODY_SIDE_RIGHT;
					}
				}
				if (cursorData->IsGestureFired(PXCCursorData::CURSOR_HAND_CLOSING, gesture)) {
					bgFrame->gestureHandClosing = true;
					if (gesture.handId == BodySideLeftID) {
						bgFrame->bodySideHandClosing = PXCCursorData::BodySideType::BODY_SIDE_LEFT;
					}
					else if (gesture.handId == BodySideRightID) {
						bgFrame->bodySideHandClosing = PXCCursorData::BodySideType::BODY_SIDE_RIGHT;
					}
				}
				if (cursorData->IsGestureFired(PXCCursorData::CURSOR_HAND_OPENING, gesture)) {
					bgFrame->gestureHandOpening = true;
					if (gesture.handId == BodySideLeftID) {
						bgFrame->bodySideHandOpening = PXCCursorData::BodySideType::BODY_SIDE_LEFT;
					}
					else if (gesture.handId == BodySideRightID) {
						bgFrame->bodySideHandOpening = PXCCursorData::BodySideType::BODY_SIDE_RIGHT;
					}
				}

				int nAlertsNumber = cursorData->QueryFiredAlertsNumber();
				for (int i = 0; i < nAlertsNumber; ++i) {
					PXCCursorData::AlertData data;
					status = cursorData->QueryFiredAlertData(i, data);
					if (status == pxcStatus::PXC_STATUS_NO_ERROR) {
						switch (data.label) {
						case PXCCursorData::AlertType::CURSOR_DETECTED:
							bgFrame->firedAlertData.Add(1);
							break;
						case PXCCursorData::AlertType::CURSOR_NOT_DETECTED:
							bgFrame->firedAlertData.Add(2);
							break;
						case PXCCursorData::AlertType::CURSOR_INSIDE_BORDERS:
							bgFrame->firedAlertData.Add(3);
							break;
						case PXCCursorData::AlertType::CURSOR_OUT_OF_BORDERS:
							bgFrame->firedAlertData.Add(4);
							break;
						case PXCCursorData::AlertType::CURSOR_TOO_CLOSE:
							bgFrame->firedAlertData.Add(5);
							break;
						case PXCCursorData::AlertType::CURSOR_TOO_FAR:
							bgFrame->firedAlertData.Add(6);
							break;
						case PXCCursorData::AlertType::CURSOR_OUT_OF_LEFT_BORDER:
							bgFrame->firedAlertData.Add(7);
							break;
						case PXCCursorData::AlertType::CURSOR_OUT_OF_RIGHT_BORDER:
							bgFrame->firedAlertData.Add(8);
							break;
						case PXCCursorData::AlertType::CURSOR_OUT_OF_TOP_BORDER:
							bgFrame->firedAlertData.Add(9);
							break;
						case PXCCursorData::AlertType::CURSOR_OUT_OF_BOTTOM_BORDER:
							bgFrame->firedAlertData.Add(10);
							break;
						case PXCCursorData::AlertType::CURSOR_ENGAGED:
							bgFrame->firedAlertData.Add(11);
							break;
						case PXCCursorData::AlertType::CURSOR_DISENGAGED:
							bgFrame->firedAlertData.Add(12);
							break;
						}
					}
				}
			}

			senseManager->ReleaseFrame();

			// Swaps background and mid RealSenseDataFrames
			std::unique_lock<std::mutex> lockIntermediate(midFrameMutex);
			bgFrame.swap(midFrame);
		}

		if (device && bSeg3DEnabled)
		{
			// Restore the mirror mode
			device->SetMirrorMode(MirrorMode);
		}

		senseManager->Close();
	}
}

// If it is not already running, starts a new camera processing thread
void RealSenseImpl::StartCamera() 
{
	if (bCameraThreadRunning == false) {
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
	if (bSeg3DEnabled)
	{
		senseManager->Enable3DSeg();
		p3DSeg = std::unique_ptr<PXC3DSeg, RealSenseDeleter>(senseManager->Query3DSeg());

		// Adding the selected stream to the StreamProfile filter Set.
		// If the camera supports HDR and confidence then these profiles are prioritized on the middleware.	
		PXCCapture::Device::StreamProfileSet profiles = {}; //This is important to initialize all the fields.
		profiles.color.imageInfo.width = colorResolution.width;
		profiles.color.imageInfo.height = colorResolution.height;

		profiles.depth.imageInfo.width = depthResolution.width;
		profiles.depth.imageInfo.height = depthResolution.height;

		// adds the profile to the stream profile Set to filter when init is called.  
		senseManager->QueryCaptureManager()->FilterByStreamProfiles(&profiles);

		// enable CPU / GPU processing
		p3DSeg->QueryInstance<PXCVideoModule>()->SetGPUExec();
	}
	if (bHandCursorEnabled) {
		senseManager->EnableHandCursor();
		pHandCursor = std::unique_ptr<PXCHandCursorModule, RealSenseDeleter>(senseManager->QueryHandCursor());
		// Get an instance of PXCCursorConfiguration
		PXCCursorConfiguration* cursorConfig = pHandCursor->CreateActiveConfiguration();
		// Make configuration changes and apply them
		cursorConfig->EnableEngagement(true);
		cursorConfig->EnableAllGestures();
		cursorConfig->EnableAllAlerts();
		cursorConfig->ApplyChanges(); // Changes only take effect when you call ApplyChanges
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
	case RealSenseFeature::SEGMENTATION_3D:
		bSeg3DEnabled = true;
		return;
	case RealSenseFeature::HAND_CURSOR:
		bHandCursorEnabled = true;
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
	case RealSenseFeature::SEGMENTATION_3D:
		bSeg3DEnabled = false;
		return;
	case RealSenseFeature::HAND_CURSOR:
		bHandCursorEnabled = false;
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

	m_status = senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_COLOR, 
										colorResolution.width, 
										colorResolution.height, 
										colorResolution.fps);

	assert(m_status == PXC_STATUS_NO_ERROR);

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
	m_status = senseManager->EnableStream(PXCCapture::StreamType::STREAM_TYPE_DEPTH, 
										depthResolution.width, 
										depthResolution.height, 
										depthResolution.fps);

	assert(m_status == PXC_STATUS_NO_ERROR);

	if (m_status == PXC_STATUS_NO_ERROR) {
		const uint32 depthImageSize = depthResolution.width * depthResolution.height;
		bgFrame->depthImage.SetNumZeroed(depthImageSize);
		midFrame->depthImage.SetNumZeroed(depthImageSize);
		fgFrame->depthImage.SetNumZeroed(depthImageSize);
	}
}

// Enables the color camera stream of the SenseManager using the specified resolution
// and resizes the colorImage and depthImage buffer of the RealSenseDataFrames to match.
void RealSenseImpl::Set3DSegCameraResolution(E3DSegResolution resolution)
{
	colorResolution = GetE3DSegResolutionValue(resolution);

	const uint8 bytesPerPixel = 4;
	const uint32 colorImageSize = colorResolution.width * colorResolution.height * bytesPerPixel;
	bgFrame->colorImage.SetNumZeroed(colorImageSize);
	midFrame->colorImage.SetNumZeroed(colorImageSize);
	fgFrame->colorImage.SetNumZeroed(colorImageSize);

	depthResolution = { 640, 480, 30.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	const uint32 depthImageSize = depthResolution.width * depthResolution.height;
	bgFrame->depthImage.SetNumZeroed(depthImageSize);
	midFrame->depthImage.SetNumZeroed(depthImageSize);
	fgFrame->depthImage.SetNumZeroed(depthImageSize);
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

	return (m_device->IsStreamProfileSetValid(&profiles) != 0);
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

	m_status = p3DScan->SetConfiguration(config);
	assert(m_status == PXC_STATUS_NO_ERROR);
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

	m_status = p3DScan->SetArea(area);
	assert(m_status == PXC_STATUS_NO_ERROR);
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
