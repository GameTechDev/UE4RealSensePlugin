#include "RealSensePluginPrivatePCH.h"
#include "BlobTrackingComponent.h"

UBlobTrackingComponent::UBlobTrackingComponent(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit) 
{ 
	m_feature = RealSenseFeature::BLOB_TRACKING;
}

void UBlobTrackingComponent::InitializeComponent()
{
	Super::InitializeComponent();

	BlobCount = 0;
	BlobCenter = FVector(0.0f, 0.0f, 0.0f);
	BlobClosest = FVector(0.0f, 0.0f, 0.0f);
	BlobTop = FVector(0.0f, 0.0f, 0.0f);
	BlobBottom = FVector(0.0f, 0.0f, 0.0f);
	BlobLeft = FVector(0.0f, 0.0f, 0.0f);
	BlobRight = FVector(0.0f, 0.0f, 0.0f);
}

void UBlobTrackingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	                                 FActorComponentTickFunction *ThisTickFunction) 
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		return;
	}

	BlobCount = globalRealSenseSession->GetBlobCount();
	BlobCenter = globalRealSenseSession->GetBlobCenter();
	BlobClosest = globalRealSenseSession->GetBlobClosest();
	BlobTop = globalRealSenseSession->GetBlobTop();
	BlobBottom = globalRealSenseSession->GetBlobBottom();
	BlobLeft = globalRealSenseSession->GetBlobLeft();
	BlobRight = globalRealSenseSession->GetBlobRight();
}
