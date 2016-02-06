#pragma once

#include "RealSenseComponent.h"
#include "CameraStreamComponent.generated.h"

// This component provides access to a buffer of RGB camera data and 
// a buffer of depth camera data, as well as convenient Texture objects
// for displaying this data.
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = RealSense) 
class UCameraStreamComponent : public URealSenseComponent
{
	GENERATED_UCLASS_BODY()

	// Array of RGBA color values representing a single frame of video captured 
	// by the RealSense RGB camera at the resolution specified by calling 
	// SetColorCameraResolution(). 
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	TArray<FSimpleColor> ColorBuffer;

	// Array of depth values (in millimeters) stored as a single frame of video 
	// captured by the RealSense depth camera at the resolution specified by 
	// calling SetDepthCameraResolution(). 
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	TArray<int32> DepthBuffer;

	// Texture2D object used to easily visualize the ColorBuffer. 
	// This texture is initialized upon setting the color camera resolution, and 
	// should be set by calling ColorBufferToTexture().
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	UTexture2D* ColorTexture;

	// Texture2D object used to easily visualize the DepthBuffer. 
	// This texture is initialized upon setting the color camera resolution, and 
	// should be set by calling DepthBufferToTexture().
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	UTexture2D* DepthTexture;

	// Enabling this feature will instruct the camera processing thread to start / 
	// resume processing of this feature.
	UFUNCTION(BlueprintCallable, Category = "RealSense")
	void EnableFeature();

	// Disabling this feature will instruct the camera processing thread to pause 
	// processing of this feature.
	UFUNCTION(BlueprintCallable, Category = "RealSense")
	void DisableFeature();

	// Sets the resolution that the RealSense RGB camera should use. 
	// This function must be called before StartCamera() in order to 
	// enable the RGB camera.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	virtual void SetColorCameraResolution(EColorResolution Resolution) override;

	// Sets the resolution that the RealSense depth camera should use. 
	// This function must be called before StartCamera() in order to 
	// enable the depth camera.
	UFUNCTION(BlueprintCallable, Category = "RealSense")
	virtual void SetDepthCameraResolution(EDepthResolution Resolution) override;

	UCameraStreamComponent();

	void InitializeComponent() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, 
					   FActorComponentTickFunction *ThisTickFunction) override;
};
