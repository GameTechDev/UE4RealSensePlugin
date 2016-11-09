#include "RealSensePluginPrivatePCH.h"
#include "HandCursorComponent.h"


UHandCursorComponent::UHandCursorComponent(const class FObjectInitializer& ObjInit)
	: Super(ObjInit) 
{ 
	HandCursorData = FVector::ZeroVector;
	IsHandCursorDataOk = false;

	m_feature = RealSenseFeature::HAND_CURSOR;
}

void UHandCursorComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UHandCursorComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	                                 FActorComponentTickFunction *ThisTickFunction) 
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		if (!HandCursorData.IsZero()) {
			HandCursorData = FVector::ZeroVector;
			OnHandCursorData.Broadcast(HandCursorData);
		}
		return;
	}

	HandCursorData = globalRealSenseSession->GetCursorData();
	IsHandCursorDataOk = globalRealSenseSession->IsCursorDataOk();
	if (IsHandCursorDataOk) {
		OnHandCursorData.Broadcast(HandCursorData);
	}
}
