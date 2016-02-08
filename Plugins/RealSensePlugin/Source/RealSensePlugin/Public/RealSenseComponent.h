#pragma once

#include "RealSenseSessionManager.h"
#include "RealSenseTypes.h"
#include "RealSenseComponent.generated.h"

// Base Class for all RealSense Components
// This class maintains basic camera information and common functions
// that all RealSense components need, such as StartCamera and 
// StopCamera and setting the color and depth resolutions.
UCLASS() 
class URealSenseComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	// Horizontal Field of View of the RealSense RGB Camera
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	float ColorHorizontalFOV;

	// Vertical Field of View of the RealSense RGB Camera
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	float ColorVerticalFOV;

	// Horizontal Field of View of the RealSense Depth Camera
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	float DepthHorizontalFOV;

	// Vertical Field of View of the RealSense Depth Camera
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	float DepthVerticalFOV;

	// Model of the connected RealSense device (F200 or R200)
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	ECameraModel CameraModel;

	// Firmware version of the connected RealSense device 
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	FString CameraFirmware;

	// This function initiates a RealSense camera processing thread that collects 
	// camera data, such as raw color and depth images and middleware-specific 
	// constructs. You should call this function after setting the color and/or depth 
	// camera resolutions.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	void StartCamera();

	// This function terminates the RealSense camera processing thread. Once terminated,
	// RealSense data will no longer be updated until after another call to 
	// StartCamera().
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	void StopCamera();

	// Enabling this feature will instruct the camera processing thread to start / 
	// resume processing of this feature.
	UFUNCTION(BlueprintCallable, Category = "RealSense")
	void EnableFeature();

	// Disabling this feature will instruct the camera processing thread to pause 
	// processing of this feature.
	UFUNCTION(BlueprintCallable, Category = "RealSense")
	void DisableFeature();

	// Returns true if the RealSense camera processing thread is currently running.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RealSense") 
	bool IsCameraRunning();

	// Returns the color camera resolution as an FStreamResolution object: 
	// width, height, fps, and pixel format.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RealSense") 
	FStreamResolution GetColorCameraResolution();

	// Sets the color camera resolution from an enumerated set of resolution options.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	virtual void SetColorCameraResolution(EColorResolution resolution);

	// Returns the depth camera resolution as an FStreamResolution object: 
	// width, height, fps, and pixel format.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RealSense") 
	FStreamResolution GetDepthCameraResolution();

	// Sets the depth camera resolution from an enumerated set of resolution options.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	virtual void SetDepthCameraResolution(EDepthResolution resolution);

	// Returns true if the combination of color and depth camera resolutions can be 
	// used together. Call this function before setting either the color and depth 
	// camera resolutions to ensure that there will be no errors initializing the 
	// RealSense camera processing thread.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	bool IsStreamSetValid(EColorResolution ColorResolution, 
						  EDepthResolution DepthResolution);
	
	URealSenseComponent();

	void InitializeComponent() override;

	void BeginPlay() override;
	
protected:
	// Reference to the global RealSenseSessionmanager actor
	ARealSenseSessionManager* globalRealSenseSession;

	RealSenseFeature m_feature;
};
