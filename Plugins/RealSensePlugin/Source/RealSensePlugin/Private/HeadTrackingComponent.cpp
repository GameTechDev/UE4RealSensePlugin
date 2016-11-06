#include "RealSensePluginPrivatePCH.h"
#include "HeadTrackingComponent.h"

UHeadTrackingComponent::UHeadTrackingComponent(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit) 
{ 
	HeadCount = 0;
	HeadPosition = FVector(0.0f, 0.0f, 0.0f);
	HeadRotation = FRotator(0.0f, 0.0f, 0.0f);

	m_feature = RealSenseFeature::HEAD_TRACKING;
}

void UHeadTrackingComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UHeadTrackingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, 
	                                 FActorComponentTickFunction *ThisTickFunction) 
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		return;
	}

	HeadCount = globalRealSenseSession->GetHeadCount();
	HeadPosition = globalRealSenseSession->GetHeadPosition();
	HeadRotation = globalRealSenseSession->GetHeadRotation();
}
