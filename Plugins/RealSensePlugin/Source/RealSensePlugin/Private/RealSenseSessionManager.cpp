#include "RealSensePluginPrivatePCH.h"
#include "RealSenseSessionManager.h"

// Initialized the feature set to 0 (no features enabled) and creates a new
// RealSenseImpl object.
ARealSenseSessionManager::ARealSenseSessionManager(const class FObjectInitializer& Init)
	: Super(Init)
{
	PrimaryActorTick.bCanEverTick = true;

	RealSenseFeatureSet = 0;

	impl = std::unique_ptr<RealSenseImpl>(new RealSenseImpl());
}

void ARealSenseSessionManager::BeginPlay() 
{
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
		const uint8 bytesPerPixel = 4;
		const uint32 ColorImageSize = impl->GetColorImageWidth() * impl->GetColorImageHeight() * bytesPerPixel;
		FMemory::Memcpy(ColorBuffer.GetData(), impl->GetColorBuffer(), ColorImageSize);

		// Update the DepthBuffer
		DepthBuffer.Empty();
		const uint32 DepthImageSize = impl->GetDepthImageWidth() * impl->GetDepthImageHeight();
		for (auto it = impl->GetDepthBuffer(), end = it + DepthImageSize; it != end; it++) {
			DepthBuffer.Add(*it);
		}
	}

	if (RealSenseFeatureSet & RealSenseFeature::SCAN_3D) {
		const uint8 bytesPerPixel = 4;
		const uint32 Scan3DImageSize = impl->GetScan3DImageWidth() * impl->GetScan3DImageHeight();
		if (impl->HasScan3DImageSizeChanged()) {
			ScanBuffer.SetNumUninitialized(Scan3DImageSize);
		}
	
		// Update the ScanBuffer
		if (ScanBuffer.Num() == Scan3DImageSize) {
			FMemory::Memcpy(ScanBuffer.GetData(), impl->GetScanBuffer(), Scan3DImageSize * bytesPerPixel);
		}
	}
}

void ARealSenseSessionManager::EnableFeature(RealSenseFeature feature)
{
	RealSenseFeatureSet |= feature;
	impl->EnableFeature(feature);
}

void ARealSenseSessionManager::DisableFeature(RealSenseFeature feature)
{
	RealSenseFeatureSet &= ~feature;
	impl->DisableFeature(feature);
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

int32 ARealSenseSessionManager::GetColorImageWidth() const
{ 
	return impl->GetColorImageWidth(); 
}

int32 ARealSenseSessionManager::GetColorImageHeight() const
{ 
	return impl->GetColorImageHeight(); 
}

int32 ARealSenseSessionManager::GetDepthImageWidth() const
{ 
	return impl->GetDepthImageWidth(); 
}

int32 ARealSenseSessionManager::GetDepthImageHeight() const
{ 
	return impl->GetDepthImageHeight(); 
}

int32 ARealSenseSessionManager::GetScan3DImageWidth() const
{ 
	return impl->GetScan3DImageWidth(); 
}

int32 ARealSenseSessionManager::GetScan3DImageHeight() const
{ 
	return impl->GetScan3DImageHeight(); 
}

float ARealSenseSessionManager::GetColorHorizontalFOV() const
{	
	return impl->GetColorHorizontalFOV(); 
}

float ARealSenseSessionManager::GetColorVerticalFOV() const
{ 
	return impl->GetColorVerticalFOV(); 
}

float ARealSenseSessionManager::GetDepthHorizontalFOV() const
{	
	return impl->GetDepthHorizontalFOV(); 
}

float ARealSenseSessionManager::GetDepthVerticalFOV() const
{ 
	return impl->GetDepthVerticalFOV(); 
}

ECameraModel ARealSenseSessionManager::GetCameraModel() const 
{ 
	return impl->GetCameraModel(); 
}

FString ARealSenseSessionManager::GetCameraFirmware() const
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

void ARealSenseSessionManager::ConfigureScanning(EScan3DMode ScanningMode, bool bSolidify, bool bTexture)
{
	impl->ConfigureScanning(ScanningMode, bSolidify, bTexture);
}

void ARealSenseSessionManager::StartScanning()
{
	impl->StartScanning();
}

void ARealSenseSessionManager::StopScanning()
{
	impl->StopScanning();
}

void ARealSenseSessionManager::SaveScan(EScan3DFileFormat SaveFileFormat, FString Filename)
{
	impl->SaveScan(SaveFileFormat, Filename);
}

void ARealSenseSessionManager::SetScanningVolume(FVector BoundingBox, int32 Resolution)
{
	impl->SetScanningVolume(BoundingBox, Resolution);
}

bool ARealSenseSessionManager::IsScanning() const
{
	return impl->IsScanning();
}

bool ARealSenseSessionManager::HasScan3DImageSizeChanged() const
{
	return impl->HasScan3DImageSizeChanged();
}

bool ARealSenseSessionManager::HasScanCompleted() const
{
	return impl->HasScanCompleted();
}

int ARealSenseSessionManager::GetHeadCount() const
{
	return impl->GetHeadCount();
}

FVector ARealSenseSessionManager::GetHeadPosition() const
{
	return impl->GetHeadPosition();
}

FRotator ARealSenseSessionManager::GetHeadRotation() const
{
	return impl->GetHeadRotation();
}