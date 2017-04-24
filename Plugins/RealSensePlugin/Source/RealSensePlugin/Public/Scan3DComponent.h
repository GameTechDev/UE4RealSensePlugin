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
#include "Scan3DComponent.generated.h"

UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = RealSense) 
class UScan3DComponent : public URealSenseComponent
{
	GENERATED_UCLASS_BODY()

	// Array of RGBA color values representing a preview image displaying the
	// current progress of scanning. This buffer will be empty until scanning
	// has started.
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	TArray<FSimpleColor> ScanBuffer;

	// Convenient Texture2D object used to easily visualize the ScanBuffer. 
	// This texture can be set by calling ColorBufferToTexture().
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	UTexture2D* ScanTexture;

	// Array of mesh vertices. This array is populated by the LoadScan() function.
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	TArray<FVector> Vertices;

	// Array of mesh triangles. This array is populated by the LoadScan() function.
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	TArray<int32> Triangles;

	// Array of mesh vertex colors. This array is populated by the LoadScan() function.
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") 
	TArray<FColor> Colors;
	
	// Triggered after a scan has been saved to disk. A call to SaveScan() will 
	// asynchronously save the scan. You can use this event to be notified when the 
	// scan has finished saving.
	UPROPERTY(BlueprintAssignable, Category = "RealSense") 
	FRealSenseNullaryDelegate OnScanComplete;

	// Sets the scanning mode and options for 3D Scanning. After calling this function, 
	// the scanning preview image will be available in the ScanBuffer.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	void ConfigureScanning(EScan3DMode ScanningMode, bool bSolidify);

	// Allows the scanning process to begin. Scanning will not actually begin until
	// the pre-requisites for the specified scanning mode have been met.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	void StartScanning();

	// Stops the scanning process.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	void StopScanning();

	// Stops the scanning process and asynchronously saves the scanned data to a mesh 
	// file with the specified file format and file name.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	void SaveScan(FString Filename);

	// Opens the specified .OBJ file and loads the mesh information into this 
	// component's Vertices, Triangles, and Colors arrays.
	UFUNCTION(BlueprintCallable, Category = "RealSense") 
	void LoadScan(FString Filename);

	// Returns true if the scanning is currently happening. Use this function after 
	// calling StartScanning() to know when the scanning process has begun.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RealSense") 
	bool IsScanning();

	UScan3DComponent();

	void InitializeComponent() override;
	
	void TickComponent(float DeltaTime, enum ELevelTick TickType, 
		               FActorComponentTickFunction *ThisTickFunction) override;

private:
	// Used internally to know when to listen for ScanComplete events.
	bool bHasScanStarted{ false };
};
