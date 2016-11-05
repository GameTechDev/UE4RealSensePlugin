#include "RealSensePluginPrivatePCH.h"
#include "UserSegmentationComponent.h"

UUserSegmentationComponent::UUserSegmentationComponent(const class FObjectInitializer& ObjInit)
	: Super(ObjInit) 
{ 
	m_feature = RealSenseFeature::SEGMENTATION_3D;
}

// Adds the CAMERA_STREAMING feature to the RealSenseSessionManager and
// initializes the ColorTexture and DepthTexture objects.
void UUserSegmentationComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ColorTexture = UTexture2D::CreateTransient(1, 1, EPixelFormat::PF_B8G8R8A8);
	DepthTexture = UTexture2D::CreateTransient(1, 1, EPixelFormat::PF_B8G8R8A8);
}

// Copies the ColorBuffer and DepthBuffer from the RealSenseSessionManager.
void UUserSegmentationComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	                                       FActorComponentTickFunction *ThisTickFunction)
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		return;
	}

	ColorBuffer = globalRealSenseSession->GetColorBuffer();
}

// If the supplied resolution is valid, this function will pass that resolution
// along to the RealSenseSessionManager and recreate the ColorTexture and DepthTexture objects
// to have the same resolution.
void UUserSegmentationComponent::Set3DSegCameraResolution(E3DSegResolution resolution)
{
	if (resolution == E3DSegResolution::UNDEFINED) {
		return;
	}

	Super::Set3DSegCameraResolution(resolution);

	int ColorImageWidth = globalRealSenseSession->GetColorImageWidth();
	int ColorImageHeight = globalRealSenseSession->GetColorImageHeight();
	ColorTexture = UTexture2D::CreateTransient(ColorImageWidth, ColorImageHeight, PF_B8G8R8A8);
	ColorTexture->UpdateResource();
}
