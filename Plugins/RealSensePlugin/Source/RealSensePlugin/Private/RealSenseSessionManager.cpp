#include "RealSensePluginPrivatePCH.h"
#include "RealSenseSessionManager.h"

// Initialized the feature set to 0 (no features enabled) and creates a new
// RealSenseImpl object.
ARealSenseSessionManager::ARealSenseSessionManager(const class FObjectInitializer& Init) : Super(Init)
{
	PrimaryActorTick.bCanEverTick = true;

	RealSenseFeatureSet = 0;

	impl = std::unique_ptr<RealSenseImpl>(new RealSenseImpl());
}

// Enable the current set of specified features
void ARealSenseSessionManager::BeginPlay() 
{
	impl->EnableRealSenseFeatures(RealSenseFeatureSet);
}

// Grab a new frame of RealSense data and process it based on the current
// set of enabled features.
void ARealSenseSessionManager::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);

	if (impl->IsCameraThreadRunning() == false) {
		return;
	}

	// Grab the next frame of RealSense data
	impl->SwapFrames();

	if (RealSenseFeatureSet & RealSenseFeature::CAMERA_STREAMING) {
		// Update the ColorBuffer
		FMemory::Memcpy(ColorBuffer.GetData(), impl->GetColorBuffer(), impl->GetColorImageWidth() * impl->GetColorImageHeight() * 4);

		// Update the DepthBuffer
		DepthBuffer.Empty();
		for (auto it = impl->GetDepthBuffer(), end = it + impl->GetDepthImageWidth() * impl->GetDepthImageHeight(); it != end; it++)
			DepthBuffer.Add(*it);
	}

	if (RealSenseFeatureSet & RealSenseFeature::SCAN_3D) {
		if (impl->HasScan3DImageSizeChanged())
			ScanBuffer.SetNumUninitialized(impl->GetScan3DImageWidth() * impl->GetScan3DImageHeight());
	
		// Update the ScanBuffer
		if (ScanBuffer.Num() == impl->GetScan3DImageWidth() * impl->GetScan3DImageHeight()) {
			FMemory::Memcpy(ScanBuffer.GetData(), impl->GetScanBuffer(), impl->GetScan3DImageWidth() * impl->GetScan3DImageHeight() * 4);
		}
	}
}

void ARealSenseSessionManager::EndPlay(const EEndPlayReason::Type EndPlayReason) 
{ 
}

void ARealSenseSessionManager::AddRealSenseFeature(RealSenseFeature feature) 
{
	RealSenseFeatureSet |= feature;
}

bool ARealSenseSessionManager::IsCameraConnected() const
{ 
	return impl->IsCameraConnected(); 
}

bool ARealSenseSessionManager::IsCameraRunning() const
{
	return impl->IsCameraThreadRunning();
}

void ARealSenseSessionManager::StartCamera() 
{ 
	impl->StartCamera(); 
}

void ARealSenseSessionManager::StopCamera() 
{ 
	impl->StopCamera(); 
}

int ARealSenseSessionManager::GetColorImageWidth() const
{ 
	return impl->GetColorImageWidth(); 
}

int ARealSenseSessionManager::GetColorImageHeight() const
{ 
	return impl->GetColorImageHeight(); 
}

int ARealSenseSessionManager::GetDepthImageWidth() const
{ 
	return impl->GetDepthImageWidth(); 
}

int ARealSenseSessionManager::GetDepthImageHeight() const
{ 
	return impl->GetDepthImageHeight(); 
}

int ARealSenseSessionManager::GetScan3DImageWidth() const
{ 
	return impl->GetScan3DImageWidth(); 
}

int ARealSenseSessionManager::GetScan3DImageHeight() const
{ 
	return impl->GetScan3DImageHeight(); 
}

const float ARealSenseSessionManager::GetColorHorizontalFOV() const
{	
	return impl->GetColorHorizontalFOV(); 
}

const float ARealSenseSessionManager::GetColorVerticalFOV() const
{ 
	return impl->GetColorVerticalFOV(); 
}

const float ARealSenseSessionManager::GetDepthHorizontalFOV() const
{	
	return impl->GetDepthHorizontalFOV(); 
}

const float ARealSenseSessionManager::GetDepthVerticalFOV() const
{ 
	return impl->GetDepthVerticalFOV(); 
}

const ECameraModel ARealSenseSessionManager::GetCameraModel() const 
{ 
	return impl->GetCameraModel(); 
}

const FString ARealSenseSessionManager::GetCameraFirmware() const
{ 
	return impl->GetCameraFirmware(); 
}

// Sets the color camera resolution and resizes the ColorBuffer to match
void ARealSenseSessionManager::SetColorCameraResolution(EColorResolution resolution)
{
	impl->SetColorCameraResolution(resolution);
	ColorBuffer.SetNumUninitialized(impl->GetColorImageWidth() * impl->GetColorImageHeight());
}

void ARealSenseSessionManager::SetDepthCameraResolution(EDepthResolution resolution) 
{ 
	impl->SetDepthCameraResolution(resolution); 
}

FStreamResolution ARealSenseSessionManager::GetColorCameraResolution() const
{
	return impl->GetColorCameraResolution();
}

FStreamResolution ARealSenseSessionManager::GetDepthCameraResolution() const
{
	return impl->GetDepthCameraResolution();
}

bool ARealSenseSessionManager::IsStreamSetValid(EColorResolution ColorResolution, EDepthResolution DepthResolution) const
{
	return impl->IsStreamSetValid(ColorResolution, DepthResolution);
}

TArray<FSimpleColor> ARealSenseSessionManager::GetColorBuffer() const
{ 
	return ColorBuffer; 
}

TArray<int32> ARealSenseSessionManager::GetDepthBuffer() const
{ 
	return DepthBuffer; 
}

TArray<FSimpleColor> ARealSenseSessionManager::GetScanBuffer() const 
{ 
	return ScanBuffer; 
}

void ARealSenseSessionManager::ConfigureScanning(EScan3DMode ScanningMode, bool Solidify, bool Texture)
{
	impl->ConfigureScanning(ScanningMode, Solidify, Texture);
}

void ARealSenseSessionManager::StartScanning()
{
	impl->StartScanning();
}

void ARealSenseSessionManager::StopScanning()
{
	impl->StopScanning();
}

void ARealSenseSessionManager::ResetScanning()
{
	impl->ResetScanning();
}

void ARealSenseSessionManager::SaveScan(EScan3DFileFormat SaveFileFormat, FString Filename)
{
	impl->SaveScan(SaveFileFormat, Filename);
}

void ARealSenseSessionManager::LoadScan(FString Filename, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FColor>& Colors)
{
	impl->LoadScan(Filename, Vertices, Triangles, Colors);
}

void ARealSenseSessionManager::SetScanningVolume(FVector BoundingBox, int32 Resolution)
{
	impl->SetScanningVolume(BoundingBox, Resolution);
}

bool ARealSenseSessionManager::IsScanning() const
{
	return impl->IsScanning();
}

bool ARealSenseSessionManager::HasScan3DImageSizeChanged()
{
	return impl->HasScan3DImageSizeChanged();
}

bool ARealSenseSessionManager::HasScanCompleted() const
{
	return impl->HasScanCompleted();
}
