#include "RealSensePluginPrivatePCH.h"
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

void UScan3DComponent::ConfigureScanning(EScan3DMode ScanningMode, bool Solidify)
{
	globalRealSenseSession->ConfigureScanning(ScanningMode, Solidify, true);
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

//void UScan3DComponent::SetScanningVolume(FVector BoundingBox, int32 Resolution)
//{
//	globalRealSenseSession->SetScanningVolume(BoundingBox, Resolution);
//}

bool UScan3DComponent::IsScanning() 
{
	return globalRealSenseSession->IsScanning();
}
