#include "RealSensePluginPrivatePCH.h"
#include "HeadTrackingComponent.h"

UHeadTrackingComponent::UHeadTrackingComponent(const class FObjectInitializer& ObjInit) : Super(ObjInit) 
{ 
}

void UHeadTrackingComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (globalRealSenseSession != nullptr)
		globalRealSenseSession->AddRealSenseFeature(RealSenseFeature::HEAD_TRACKING);

	HeadCount = 0;
	HeadPosition = FVector(0.0f, 0.0f, 0.0f);
	HeadRotation = FRotator(0.0f, 0.0f, 0.0f);
}

// Copies the ColorBuffer and DepthBuffer from the RealSenseSessionManager.
void UHeadTrackingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (globalRealSenseSession->IsCameraRunning() == false)
		return;

	HeadCount = globalRealSenseSession->GetHeadCount();
	HeadPosition = globalRealSenseSession->GetHeadPosition();
	HeadRotation = globalRealSenseSession->GetHeadRotation();
}

void UHeadTrackingComponent::BeginPlay()
{
//	globalRealSenseSession->StartCamera();
}

void UHeadTrackingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
//	globalRealSenseSession->StopCamera();
}
