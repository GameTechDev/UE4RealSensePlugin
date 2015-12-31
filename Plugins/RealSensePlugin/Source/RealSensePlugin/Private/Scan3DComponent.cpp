#include "RealSensePluginPrivatePCH.h"
#include "Scan3DComponent.h"

UScan3DComponent::UScan3DComponent(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit) 
{ 
	bHasScanStarted = false;
}

// Adds the SCAN_3D feature to the RealSenseSessionManager and initializes the 
// ScanTexture object.
void UScan3DComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	if (globalRealSenseSession != nullptr) {
		globalRealSenseSession->AddRealSenseFeature(RealSenseFeature::SCAN_3D);
	}

	ScanTexture = UTexture2D::CreateTransient(1, 1,	EPixelFormat::PF_B8G8R8A8);
}

// Copies the ScanBuffer and checks if a current scan has just completed.
// If it has, the OnScanComplete event is broadcast.
void UScan3DComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, 
	                                 FActorComponentTickFunction *ThisTickFunction) 
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		return;
	}

	// The 3D Scanning preview image size can be changed automatically by the
	// middleware, so it is important to check every tick if the image size
	// has changed so that the ScanTexture object can be resized to match.
	if (globalRealSenseSession->HasScan3DImageSizeChanged()) {
		int Scan3DImageWidth = globalRealSenseSession->GetScan3DImageWidth();
		int Scan3DImageHeight = globalRealSenseSession->GetScan3DImageHeight();
		ScanTexture = UTexture2D::CreateTransient(Scan3DImageWidth, Scan3DImageHeight,
											      EPixelFormat::PF_B8G8R8A8);
		ScanTexture->UpdateResource();
	}

	ScanBuffer = globalRealSenseSession->GetScanBuffer();

	if (globalRealSenseSession->HasScanCompleted() && bHasScanStarted) {
		OnScanComplete.Broadcast();
		bHasScanStarted = false;
	}
}

void UScan3DComponent::ConfigureScanning(EScan3DMode ScanningMode, bool bSolidify)
{
	globalRealSenseSession->ConfigureScanning(ScanningMode, bSolidify, false);
}

void UScan3DComponent::StartScanning()
{
	globalRealSenseSession->StartScanning();
	bHasScanStarted = true;
}

void UScan3DComponent::StopScanning()
{
	globalRealSenseSession->StopScanning();
}

void UScan3DComponent::SaveScan(FString Filename)
{
	Filename = FPaths::GameContentDir().Append(Filename);
	globalRealSenseSession->SaveScan(EScan3DFileFormat::OBJ, Filename);
}

void UScan3DComponent::LoadScan(FString Filename)
{
	Filename = FPaths::GameContentDir().Append(Filename);
	globalRealSenseSession->LoadScan(Filename, Vertices, Triangles, Colors);
}

bool UScan3DComponent::IsScanning() 
{
	return globalRealSenseSession->IsScanning();
}
