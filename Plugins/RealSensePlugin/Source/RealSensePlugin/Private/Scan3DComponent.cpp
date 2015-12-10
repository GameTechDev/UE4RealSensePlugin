#include "RealSensePluginPrivatePCH.h"
#include "ProceduralMeshActor.h"
#include "Scan3DComponent.h"

UScan3DComponent::UScan3DComponent(const class FObjectInitializer& ObjInit) : Super(ObjInit) 
{ 
}

// Adds the SCAN_3D feature to the RealSenseSessionManager
void UScan3DComponent::InitializeComponent() 
{
	Super::InitializeComponent();
	
	if (globalRealSenseSession != nullptr) 
		globalRealSenseSession->AddRealSenseFeature(RealSenseFeature::SCAN_3D);

	ScanTexture = UTexture2D::CreateTransient(1, 1,	EPixelFormat::PF_B8G8R8A8);
	ClearTexture(ScanTexture, FColor(0, 0, 0, 0));
}

void UScan3DComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Copies the ScanBuffer and checks if a current scan has just completed.
// If it has, the OnScanComplete event is broadcast.
void UScan3DComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) 
{
	if (globalRealSenseSession->IsCameraRunning() == false)
		return;

	// The 3D Scanning preview image size can be changed automatically by the
	// middleware, so it is important to check every tick if the image size
	// has changed so that the ScanTexture object can be resized to match.
	if (globalRealSenseSession->HasScan3DImageSizeChanged()) {
		ScanTexture = UTexture2D::CreateTransient(globalRealSenseSession->GetScan3DImageWidth(),
									  			  globalRealSenseSession->GetScan3DImageHeight(),
												  EPixelFormat::PF_B8G8R8A8);
		ScanTexture->UpdateResource();
	}

	ScanBuffer = globalRealSenseSession->GetScanBuffer();

	if (globalRealSenseSession->HasScanCompleted() && HasScanStarted) {
		OnScanComplete.Broadcast();
		HasScanStarted = false;
	}
}

void UScan3DComponent::ConfigureScanning(EScan3DMode ScanningMode, bool Solidify, bool Texture)
{
	globalRealSenseSession->ConfigureScanning(ScanningMode, Solidify, Texture);
}

void UScan3DComponent::StartScanning()
{
	globalRealSenseSession->StartScanning();
	HasScanStarted = true;
}

void UScan3DComponent::StopScanning()
{
	globalRealSenseSession->StopScanning();
}

void UScan3DComponent::ResetScanning()
{
	globalRealSenseSession->ResetScanning();
}

void UScan3DComponent::SaveScan(EScan3DFileFormat SaveFileFormat, FString Filename)
{
	Filename = FPaths::GameContentDir().Append(Filename);
	globalRealSenseSession->SaveScan(SaveFileFormat, Filename);
}

void UScan3DComponent::LoadScan(FString Filename)
{
	Filename = FPaths::GameContentDir().Append(Filename);
	globalRealSenseSession->LoadScan(Filename, Vertices, Triangles, Colors);
}

void UScan3DComponent::SetScanningVolume(FVector BoundingBox, int32 Resolution)
{
	globalRealSenseSession->SetScanningVolume(BoundingBox, Resolution);
}

bool UScan3DComponent::IsScanning() 
{
	return globalRealSenseSession->IsScanning();
}
/*
void MyAlertHandler::OnAlert(const PXC3DScan::AlertData& data)
{
	switch (data.label) {
	case PXC3DScan::ALERT_FACE_DETECTED:
		prereqs.FaceDetected = FaceStatusDetected::Detected;
		break;

	case PXC3DScan::ALERT_FACE_NOT_DETECTED:
		prereqs.FaceDetected = FaceStatusDetected::NotDetected;
		break;

	case PXC3DScan::ALERT_FACE_X_IN_RANGE:
		prereqs.XInRange = FaceStatusXAxis::InRange;
		break;

	case PXC3DScan::ALERT_FACE_X_TOO_FAR_LEFT:
		prereqs.XInRange = FaceStatusXAxis::TooFarLeft;
		break;

	case PXC3DScan::ALERT_FACE_X_TOO_FAR_RIGHT:
		prereqs.XInRange = FaceStatusXAxis::TooFarRight;
		break;

	case PXC3DScan::ALERT_FACE_Y_IN_RANGE:
		prereqs.YInRange = FaceStatusYAxis::InRange;
		break;

	case PXC3DScan::ALERT_FACE_Y_TOO_FAR_UP:
		prereqs.YInRange = FaceStatusYAxis::TooFarUp;
		break;

	case PXC3DScan::ALERT_FACE_Y_TOO_FAR_DOWN:
		prereqs.YInRange = FaceStatusYAxis::TooFarDown;
		break;

	case PXC3DScan::ALERT_FACE_Z_IN_RANGE:
		prereqs.ZInRange = FaceStatusZAxis::InRange;
		break;

	case PXC3DScan::ALERT_FACE_Z_TOO_CLOSE:
		prereqs.ZInRange = FaceStatusZAxis::TooClose;
		break;

	case PXC3DScan::ALERT_FACE_Z_TOO_FAR:
		prereqs.ZInRange = FaceStatusZAxis::TooFar;
		break;

	case PXC3DScan::ALERT_FACE_YAW_IN_RANGE:
		prereqs.YawInRange = FaceStatusYaw::InRange;
		break;

	case PXC3DScan::ALERT_FACE_YAW_TOO_FAR_LEFT:
		prereqs.YawInRange = FaceStatusYaw::TooFarLeft;
		break;

	case PXC3DScan::ALERT_FACE_YAW_TOO_FAR_RIGHT:
		prereqs.YawInRange = FaceStatusYaw::TooFarRight;
		break;

	case PXC3DScan::ALERT_FACE_PITCH_IN_RANGE:
		prereqs.PitchInRange = FaceStatusPitch::InRange;
		break;

	case PXC3DScan::ALERT_FACE_PITCH_TOO_FAR_UP:
		prereqs.PitchInRange = FaceStatusPitch::TooFarUp;
		break;

	case PXC3DScan::ALERT_FACE_PITCH_TOO_FAR_DOWN:
		prereqs.PitchInRange = FaceStatusPitch::TooFarDown;
		break;
	}
	
	CheckPrereqs();
}

void MyAlertHandler::CheckPrereqs()
{
	if ((prereqs.FaceDetected == FaceStatusDetected::Detected) &&
		(prereqs.XInRange == FaceStatusXAxis::InRange) &&
		(prereqs.YInRange == FaceStatusYAxis::InRange) &&
		(prereqs.ZInRange == FaceStatusZAxis::InRange) &&
		(prereqs.PitchInRange == FaceStatusPitch::InRange) &&
		(prereqs.YawInRange == FaceStatusYaw::InRange))
	{
		prereqs.Satisfied = true;
	}
	else
		prereqs.Satisfied = false;
}
*/
