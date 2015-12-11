#include "RealSensePluginPrivatePCH.h"
#include "RealSenseComponent.h"

// Specifies that this component should be initialized and can tick.
URealSenseComponent::URealSenseComponent(const class FObjectInitializer& ObjInit) : Super(ObjInit)
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;

	globalRealSenseSession = nullptr;
}

// This function attempts to find a RealSenseSessionManager actor in the world and store
// a reference to it. If the actor does not exist, a new RealSenseSessionManager is 
// spawned and a reference to it is stored.
void URealSenseComponent::InitializeComponent()
{
	if (globalRealSenseSession == nullptr) {
		for (TActorIterator<ARealSenseSessionManager> Itr(GetWorld()); Itr; ++Itr) {
			globalRealSenseSession = (ARealSenseSessionManager*)*Itr;
		}
		if (globalRealSenseSession == nullptr) {
			RS_LOG(Log, "Creating RealSenseSessionManager Actor")
				globalRealSenseSession = GetWorld()->SpawnActor<ARealSenseSessionManager>(ARealSenseSessionManager::StaticClass());
		}
	}
}

// Queries the camera model, firmware, and field of view information from the RealSense camera
void URealSenseComponent::BeginPlay()
{
	CameraModel = globalRealSenseSession->GetCameraModel();
	CameraFirmware = globalRealSenseSession->GetCameraFirmware();

	ColorHorizontalFOV = globalRealSenseSession->GetColorHorizontalFOV();
	ColorVerticalFOV = globalRealSenseSession->GetColorVerticalFOV();

	DepthHorizontalFOV = globalRealSenseSession->GetDepthHorizontalFOV();
	DepthVerticalFOV = globalRealSenseSession->GetDepthVerticalFOV();
}

bool URealSenseComponent::IsCameraRunning()
{
	return globalRealSenseSession->IsCameraRunning();
}

FStreamResolution URealSenseComponent::GetColorCameraResolution() {
	return globalRealSenseSession->GetColorCameraResolution();
}

FStreamResolution URealSenseComponent::GetDepthCameraResolution() {
	return globalRealSenseSession->GetDepthCameraResolution();
}

void URealSenseComponent::SetColorCameraResolution(EColorResolution resolution) {
	if (resolution != EColorResolution::UNDEFINED) {
		globalRealSenseSession->SetColorCameraResolution(resolution);
	}
}

void URealSenseComponent::SetDepthCameraResolution(EDepthResolution resolution) {
	if (resolution != EDepthResolution::UNDEFINED) {
		globalRealSenseSession->SetDepthCameraResolution(resolution);
	}
}

bool URealSenseComponent::IsStreamSetValid(EColorResolution CResolution, EDepthResolution DResolution) {
	return globalRealSenseSession->IsStreamSetValid(CResolution, DResolution);
}

void URealSenseComponent::StartCamera() {
	globalRealSenseSession->StartCamera();
}

void URealSenseComponent::StopCamera() {
	globalRealSenseSession->StopCamera();
}
