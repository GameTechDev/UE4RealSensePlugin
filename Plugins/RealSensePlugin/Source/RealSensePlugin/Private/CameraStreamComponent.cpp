#include "RealSensePluginPrivatePCH.h"
#include "CameraStreamComponent.h"

UCameraStreamComponent::UCameraStreamComponent(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit) 
{ 
}

// Adds the CAMERA_STREAMING feature to the RealSenseSessionManager and
// initializes the ColorTexture and DepthTexture objects.
void UCameraStreamComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (globalRealSenseSession != nullptr) {
		globalRealSenseSession->AddRealSenseFeature(RealSenseFeature::CAMERA_STREAMING);
	}

	ColorTexture = UTexture2D::CreateTransient(1, 1, EPixelFormat::PF_B8G8R8A8);
	DepthTexture = UTexture2D::CreateTransient(1, 1, EPixelFormat::PF_B8G8R8A8);
}

// Copies the ColorBuffer and DepthBuffer from the RealSenseSessionManager.
void UCameraStreamComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, 
	                                       FActorComponentTickFunction *ThisTickFunction)
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		return;
	}

	ColorBuffer = globalRealSenseSession->GetColorBuffer();
	DepthBuffer = globalRealSenseSession->GetDepthBuffer();
}

// If the supplied resolution is valid, this function will pass that resolution
// along to the RealSenseSessionManager and recreate the ColorTexture objects
// to have the same resolution.
void UCameraStreamComponent::SetColorCameraResolution(EColorResolution resolution) 
{
	if (resolution == EColorResolution::UNDEFINED) {
		return;
	}

	Super::SetColorCameraResolution(resolution);

	int ColorImageWidth = globalRealSenseSession->GetColorImageWidth();
	int ColorImageHeight = globalRealSenseSession->GetColorImageHeight();
	ColorTexture = UTexture2D::CreateTransient(ColorImageWidth, ColorImageHeight,
											   PF_B8G8R8A8);
	ColorTexture->UpdateResource();
}

// If the supplied resolution is valid, this function will pass that resolution
// along to the RealSenseSessionManager and recreate the DepthTexture objects
// to have the same resolution.
void UCameraStreamComponent::SetDepthCameraResolution(EDepthResolution resolution)
{
	if (resolution == EDepthResolution::UNDEFINED) {
		return;
	}

	Super::SetDepthCameraResolution(resolution);
	
	int DepthImageWidth = globalRealSenseSession->GetDepthImageWidth();
	int DepthImageHeight = globalRealSenseSession->GetDepthImageHeight();
	DepthTexture = UTexture2D::CreateTransient(DepthImageWidth, DepthImageHeight, 
											   PF_B8G8R8A8);
	DepthTexture->UpdateResource();
}
