#pragma once

#include "RealSenseImpl.h"
#include "RealSenseTypes.h"

#include "RealSenseSessionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRealSenseNullaryDelegate);

UCLASS(ClassGroup = RealSense)
class ARealSenseSessionManager : public AActor
{
	GENERATED_UCLASS_BODY()

	// Adds a new feature to the RealSense feature set
	void AddRealSenseFeature(RealSenseFeature feature);

	// RealSenseComponent Support

	// Starts the camera processing thread.
	void StartCamera();

	// Stops the camera processing thread.
	void StopCamera();

	// Returns true if the camera processing thread is currently executing.
	bool IsCameraRunning() const;

	// Returns true if there is a physical camera connected.
	bool IsCameraConnected() const;

	// Returns the horizontal field of view of the RealSense RGB camera.
	const float GetColorHorizontalFOV() const;

	// Returns the vertical field of view of the RealSense RGB camera.
	const float GetColorVerticalFOV() const;

	// Returns the horizontal field of view of the RealSense depth camera.
	const float GetDepthHorizontalFOV() const;

	// Returns the vertical field of view of the RealSense depth camera.
	const float GetDepthVerticalFOV() const;

	// Returns the model of the connected camera: R200, F200, or Other.
	const ECameraModel GetCameraModel() const;

	// Returns the connected camera's firmware version as a human readable string.
	const FString GetCameraFirmware() const;

	// Returns the user-defined resolution of the RealSense RGB camera.
	FStreamResolution GetColorCameraResolution() const;

	// Returns the width of the user-defined resolution of the RealSense RGB camera.
	int GetColorImageWidth() const;

	// Returns the height of the user-defined resolution of the RealSense RGB camera.
	int GetColorImageHeight() const;

	// Set the resolution to be used by the RealSense RGB camera.
	void SetColorCameraResolution(EColorResolution resolution);

	// Returns the user-defined resolution of the RealSense depth camera.
	FStreamResolution GetDepthCameraResolution() const;

	// Returns the width of the user-defined resolution of the RealSense depth camera.
	int GetDepthImageWidth() const;

	// Returns the height of the user-defined resolution of the RealSense depth camera.
	int GetDepthImageHeight() const;

	// Set the resolution to be used by the RealSense depth camera.
	void SetDepthCameraResolution(EDepthResolution resolution);

	// Returns true if the combination of RGB camera resolution and depth camera 
	// resolution is valid. Validity is determined internally by the RSSDK.
	bool IsStreamSetValid(EColorResolution ColorResolution, EDepthResolution DepthResolution) const;

	// CameraStreamComponent Support

	// Returns a pointer to the latest frame obtained from the RealSense RGB camera.
	TArray<FSimpleColor> GetColorBuffer() const;

	// Returns a pointer to the latest frame obtained from the RealSense depth camera.
	TArray<int32> GetDepthBuffer() const;

	// Scan3DComponent Support 

	// Configures the 3D Scanning middleware.
	//
	// Scanning Modes : Face, Head, Body, Object, Variable
	// If you use Variable scanning mode, you must also set the scanning volume
	// to reflect the 3D space you wish to scan.
	//
	// If solidify is true, the mesh created by the middleware will be closed.
	//
	// If texture is true, the middleware will create a texture file along with
	// the mesh. If false, the mesh file will be include vertex color information.
	void ConfigureScanning(EScan3DMode ScanningMode, bool bSolidify, bool Texture);

	// Only use this function if 3D Scanning is set to Variable mode.
	// Sets the bounding box of the 3D space in which you wish to scan
	// and the voxel resolution to use.
	void SetScanningVolume(FVector BoundingBox, int32 Resolution);

	// Instructs the 3D Scanning module to begin its attempt to scan.
	// Scanning will not actually begin until internal pre-requisites of the
	// scanning module are satisfied, such as the presence of a minimum 
	// amount of geometry.
	void StartScanning();

	// Instructs the 3D Scanning module to stop scanning.
	void StopScanning();

	// Instructs the 3D Scanning module to restart scanning.
	void ResetScanning();

	// Saves the scanned data to a file with the specified format and filename.
	// Currently only the OBJ format is supported.
	void SaveScan(EScan3DFileFormat SaveFileFormat, FString filename);

	// Loads an OBJ mesh file and parses it for vertices, triangles, 
	// and vertex color information. The input arrays will be emptied
	// and refilled as a result of this function.
	void LoadScan(FString filename, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FColor>& Colors);

	// Returns true if the 3D scanning module is currently scanning.
	bool IsScanning() const;

	// Returns the middleware-defined resolution used by the 3D scanning module.
	FStreamResolution GetScan3DResolution() const;

	// Returns the width of the middleware-defined resolution used by the 
	// 3D scanning module.
	int GetScan3DImageWidth() const;

	// Returns the height of the middleware-defined resolution used by the 
	// 3D scanning module.
	int GetScan3DImageHeight() const;

	// Returns a pointer to the latest frame obtained from the 3D scanning
	// module, representing a preview of the current scanning progress.
	TArray<FSimpleColor> GetScanBuffer() const;

	// Returns true if the resolution of the 3D scanning module has changed.
	bool HasScan3DImageSizeChanged();

	// Returns true when the 3D scanning module finishes saving a scan.
	bool HasScanCompleted() const;

	// Creates a new RealSenseImpl object to handle the RealSense implementation
	ARealSenseSessionManager();

	// Calls EnableRealSenseFeatureSet to initialize the set of enabled features
	virtual void BeginPlay() override;

	// Grabs the next frame of RealSense data and updates private members
	virtual void Tick(float DeltaSeconds) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	std::unique_ptr<RealSenseImpl> impl;

	uint32 RealSenseFeatureSet;

	TArray<FSimpleColor> ColorBuffer;
	TArray<int32> DepthBuffer;
	TArray<FSimpleColor> ScanBuffer;
};
