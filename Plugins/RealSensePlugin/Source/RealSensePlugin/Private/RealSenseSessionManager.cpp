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
//	impl->EnableRealSenseFeatures(RealSenseFeatureSet);
}

// Grab a new frame of RealSense data and process it based on the current
// set of enabled features.
void ARealSenseSessionManager::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);

	if (impl->IsCameraThreadRunning() == false)
		return;

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
		FMemory::Memcpy(ScanBuffer.GetData(), impl->GetScanBuffer(), impl->GetScan3DImageWidth() * impl->GetScan3DImageHeight() * 4);
	}

	if (RealSenseFeatureSet & RealSenseFeature::HEAD_TRACKING) {
	}
}

void ARealSenseSessionManager::EndPlay(const EEndPlayReason::Type EndPlayReason) 
{ 
}

void ARealSenseSessionManager::AddRealSenseFeature(RealSenseFeature feature) 
{
	RealSenseFeatureSet |= feature;
	impl->EnableRealSenseFeature(feature);
}

bool ARealSenseSessionManager::IsCameraConnected() 
{ 
	return impl->IsCameraConnected(); 
}

bool ARealSenseSessionManager::IsCameraRunning()
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

const int ARealSenseSessionManager::GetColorImageWidth() 
{ 
	return impl->GetColorImageWidth(); 
}

const int ARealSenseSessionManager::GetColorImageHeight() 
{ 
	return impl->GetColorImageHeight(); 
}

const int ARealSenseSessionManager::GetDepthImageWidth() 
{ 
	return impl->GetDepthImageWidth(); 
}

const int ARealSenseSessionManager::GetDepthImageHeight() 
{ 
	return impl->GetDepthImageHeight(); 
}

const int ARealSenseSessionManager::GetScan3DImageWidth() 
{ 
	return impl->GetScan3DImageWidth(); 
}

const int ARealSenseSessionManager::GetScan3DImageHeight() 
{ 
	return impl->GetScan3DImageHeight(); 
}

const float ARealSenseSessionManager::GetColorHorizontalFOV() 
{	
	return impl->GetColorHorizontalFOV(); 
}

const float ARealSenseSessionManager::GetColorVerticalFOV() 
{ 
	return impl->GetColorVerticalFOV(); 
}

const float ARealSenseSessionManager::GetDepthHorizontalFOV() 
{	
	return impl->GetDepthHorizontalFOV(); 
}

const float ARealSenseSessionManager::GetDepthVerticalFOV() 
{ 
	return impl->GetDepthVerticalFOV(); 
}

const ECameraModel ARealSenseSessionManager::GetCameraModel() 
{ 
	return impl->GetCameraModel(); 
}

const FString ARealSenseSessionManager::GetCameraFirmware() 
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

FStreamResolution ARealSenseSessionManager::GetColorCameraResolution() 
{
	return impl->GetColorCameraResolution();
}

FStreamResolution ARealSenseSessionManager::GetDepthCameraResolution() 
{
	return impl->GetDepthCameraResolution();
}

bool ARealSenseSessionManager::IsStreamSetValid(EColorResolution ColorResolution, EDepthResolution DepthResolution) 
{
	return impl->IsStreamSetValid(ColorResolution, DepthResolution);
}

TArray<FSimpleColor> ARealSenseSessionManager::GetColorBuffer() 
{ 
	return ColorBuffer; 
}

TArray<int32> ARealSenseSessionManager::GetDepthBuffer() 
{ 
	return DepthBuffer; 
}

int32 ARealSenseSessionManager::GetHeadCount()
{
	return impl->GetHeadCount();
}

FVector ARealSenseSessionManager::GetHeadPosition()
{
	FVector Position = impl->GetHeadPosition();
	return FVector(Position.Z, Position.X, Position.Y);
}

FRotator ARealSenseSessionManager::GetHeadRotation()
{
	return impl->GetHeadRotation();
}

TArray<FSimpleColor> ARealSenseSessionManager::GetScanBuffer()
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

bool ARealSenseSessionManager::IsScanning()
{
	return impl->IsScanning();
}

bool ARealSenseSessionManager::HasScan3DImageSizeChanged()
{
	return impl->HasScan3DImageSizeChanged();
}

bool ARealSenseSessionManager::HasScanCompleted()
{
	return impl->HasScanCompleted();
}
