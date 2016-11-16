#include "RealSensePluginPrivatePCH.h"
#include "HandCursorComponent.h"


UHandCursorComponent::UHandCursorComponent(const class FObjectInitializer& ObjInit)
	: Super(ObjInit) 
{ 
	HandCursorData = FVector::ZeroVector;
	IsHandCursorDataValid = false;
	HandCursorDataLeft = FVector::ZeroVector;
	IsHandCursorDataLeftValid = false;
	HandCursorDataRight = FVector::ZeroVector;
	IsHandCursorDataRightValid = false;
	StatusCode = EStatus::PXC_STATUS_NO_ERROR;

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
		HandCursorData = FVector::ZeroVector;
		IsHandCursorDataValid = false;
		HandCursorDataLeft = FVector::ZeroVector;
		IsHandCursorDataLeftValid = false;
		HandCursorDataRight = FVector::ZeroVector;
		IsHandCursorDataRightValid = false;
		return;
	}

	HandCursorData = globalRealSenseSession->GetCursorData();
	IsHandCursorDataValid = globalRealSenseSession->IsCursorDataValid();
	if (IsHandCursorDataValid) {
		OnHandCursorData.Broadcast(HandCursorData);
	}

	HandCursorDataLeft = globalRealSenseSession->GetCursorDataLeft();
	IsHandCursorDataLeftValid = globalRealSenseSession->IsCursorDataLeftValid();
	if (IsHandCursorDataLeftValid) {
		OnHandCursorDataLeft.Broadcast(HandCursorDataLeft);
	}

	HandCursorDataRight = globalRealSenseSession->GetCursorDataRight();
	IsHandCursorDataRightValid = globalRealSenseSession->IsCursorDataRightValid();
	if (IsHandCursorDataRightValid) {
		OnHandCursorDataRight.Broadcast(HandCursorDataRight);
	}

	if (globalRealSenseSession->IsGestureClick()) {
		OnGestureClick.Broadcast(EBodySideType(globalRealSenseSession->GetBodySideClick()));
	}

	if (globalRealSenseSession->IsGestureClockwiseCircle()) {
		OnGestureClockwiseCircle.Broadcast(EBodySideType(globalRealSenseSession->GetBodySideClockwiseCircle()));
	}

	if (globalRealSenseSession->IsGestureCounterClockwiseCircle()) {
		OnGestureCounterClockwiseCircle.Broadcast(EBodySideType(globalRealSenseSession->GetBodySideCounterClockwiseCircle()));
	}

	if (globalRealSenseSession->IsGestureHandClosing()) {
		OnGestureHandClosing.Broadcast(EBodySideType(globalRealSenseSession->GetBodySideHandClosing()));
	}

	if (globalRealSenseSession->IsGestureHandOpening()) {
		OnGestureHandOpening.Broadcast(EBodySideType(globalRealSenseSession->GetBodySideHandOpening()));
	}

	TArray<int> aAlertData = globalRealSenseSession->GetFiredAlertData();
	if (aAlertData.Num() > 0) {
		m_Alerts.Empty();
		for (int i = 0; i < aAlertData.Num(); ++i) {
			m_Alerts.Add(EAlertType(aAlertData[i]));
		}
		OnFiredAlertData.Broadcast(m_Alerts);
	}

	StatusCode = EStatus(globalRealSenseSession->GetStatusCode());
	if (StatusCode != EStatus::PXC_STATUS_NO_ERROR) {
		OnStatusCode.Broadcast(StatusCode);
	}
}
