/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
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

	// Enable 3D segmentation
	// This function must be called before StartCamera() in order to
	// enable 3D segmentation.
	UFUNCTION(BlueprintCallable, Category = "RealSense")
	virtual void Enable3DSegmentation(bool b3DSeg);

	UCameraStreamComponent();

	void InitializeComponent() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, 
					   FActorComponentTickFunction *ThisTickFunction) override;
};
