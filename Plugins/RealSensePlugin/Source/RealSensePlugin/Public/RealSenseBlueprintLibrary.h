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

#include "RealSenseTypes.h"
#include "RealSenseUtils.h"
#include "RealSenseBlueprintLibrary.generated.h"

UCLASS() 
class URealSenseBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	// Returns a string representation of the input ColorResolution
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RealSense Utilities") 
	static FString EColorResolutionToString(EColorResolution value);

	// Returns a string representation of the input DepthResolution
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RealSense Utilities") 
	static FString EDepthResolutionToString(EDepthResolution value);

	// Returns a string representation of the input CameraModel
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RealSense Utilities") 
	static FString ECameraModelToString(ECameraModel value);

	// Fills a Texture2D object with the data from a buffer of FSimpleColors.
	// This function will return null if the size of the input buffer does not
	// match the resolution of the Texture2D object.
	// @param Buffer - TArray of FSimpleColor values (RGBA)
	// @param Texture - Texture2D object to fill with data
	// @return The input Texture2D object, modified to contain the data from the 
	// input buffer
	UFUNCTION(BlueprintCallable, Category = "RealSense Utilities") 
	static UTexture2D* ColorBufferToTexture(const TArray<FSimpleColor>& Buffer, 
											UTexture2D* Texture);

	// Fills a Texture2D object with the data from a buffer of integers 
	// (representing depth values).
	// This function will return null if the size of the input buffer does not
	// match the resolution of the Texture2D object.
	// @param Buffer - TArray of integer values (depth in millimeters)
	// @param Texture - Texture2D object to fill with data
	// @return The input Texture2D object, modified to contain the data from the 
	// input buffer
	UFUNCTION(BlueprintCallable, Category = "RealSense Utilities") 
	static UTexture2D* DepthBufferToTexture(const TArray<int32>& Buffer, 
											UTexture2D* Texture);

	// Returns an array of .OBJ filenames found in the specified directory.
	// Note: The path is relative to the /Game/Content asset directory.
	// Example: GetMeshFiles("Scans/Faces") searches for .OBJ files in 
	// /Games/Content/Scans/Faces.
	UFUNCTION(BlueprintCallable, Category = "RealSense Utilities") 
	static TArray<FString> GetMeshFiles(FString Directory);
};