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

#include "RealSensePluginPrivatePCH.h"
#include "RealSenseBlueprintLibrary.h"

URealSenseBlueprintLibrary::URealSenseBlueprintLibrary(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit) 
{ 
}

FString URealSenseBlueprintLibrary::EColorResolutionToString(EColorResolution value) 
{
	switch (value) {
	case EColorResolution::RES1:
		return FString("1920 x 1080 x 30");
	case EColorResolution::RES2:
		return FString("1280 x 720 x 30");
	case EColorResolution::RES3:
		return FString("640 x 480 x 60");
	case EColorResolution::RES4:
		return FString("640 x 480 x 30");
	case EColorResolution::RES5:
		return FString("320 x 240 x 60");
	case EColorResolution::RES6:
		return FString("320 x 240 x 30");
	default:
		return FString(" ");
	}
}

FString URealSenseBlueprintLibrary::EDepthResolutionToString(EDepthResolution value) 
{
	switch (value) {
	case EDepthResolution::RES1:
		return FString("640 x 480 x 60 (F200)");
	case EDepthResolution::RES2:
		return FString("640 x 480 x 30 (F200)");
	case EDepthResolution::RES3:
		return FString("628 x 468 x 90 (R200)");
	case EDepthResolution::RES4:
		return FString("628 x 468 x 60 (R200)");
	case EDepthResolution::RES5:
		return FString("628 x 468 x 30 (R200)");
	case EDepthResolution::RES6:
		return FString("480 x 360 x 90 (R200)");
	case EDepthResolution::RES7:
		return FString("480 x 360 x 60 (R200)");
	case EDepthResolution::RES8:
		return FString("480 x 360 x 30 (R200)");
	case EDepthResolution::RES9:
		return FString("320 x 240 x 90 (R200)");
	case EDepthResolution::RES10:
		return FString("320 x 240 x 60 (R200)");
	case EDepthResolution::RES11:
		return FString("320 x 240 x 30 (R200)");
	default:
		return FString(" ");
	}
}

FString URealSenseBlueprintLibrary::ECameraModelToString(ECameraModel value) 
{
	switch (value) {
	case ECameraModel::F200:
		return FString("Front-Facing (F200)");
	case ECameraModel::R200:
		return FString("World-Facing (R200)");
	case ECameraModel::SR300:
		return FString("Short-Range (SR300)");
	case ECameraModel::Other:
		return FString("Unknown Camera Model");
	default:
		return FString(" ");
	}
}

// Copies the data from the input Buffer into the PlatformData of the Texture object.
// For convenience, this function returns a pointer to the input Texture that was 
// modified.
UTexture2D* URealSenseBlueprintLibrary::ColorBufferToTexture(const TArray<FSimpleColor>& Buffer, UTexture2D* Texture) 
{
	if (Texture == nullptr) {
		return nullptr;
	}

	// Test that the Buffer and Texture have the same capacity
	if (Buffer.Num() != Texture->GetSizeX() * Texture->GetSizeY()) {
		return nullptr;
	}
	
	// The Texture's PlatformData needs to be locked before it can be modified.
	auto out = reinterpret_cast<uint8*>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	// There are four bytes per pixel, one each for Red, Green, Blue, and Alpha.
	uint8 bytesPerPixel = 4;
	uint32 size = Texture->GetSizeX() * Texture->GetSizeY() * bytesPerPixel;
	memcpy_s(out, size, Buffer.GetData(), size);

	Texture->PlatformData->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

// Copies the data from the input Buffer into the PlatformData of the Texture object.
// For convenience, this function returns a pointer to the input Texture that was modified.
UTexture2D* URealSenseBlueprintLibrary::DepthBufferToTexture(const TArray<int32>& Buffer, UTexture2D* Texture)
{
	if (Texture == nullptr) {
		return nullptr;
	}

	// Test that the Buffer and Texture have the same capacity
	if (Buffer.Num() != Texture->GetSizeX() * Texture->GetSizeY()) {
		return nullptr;
	}

	// The Texture's PlatformData needs to be locked before it can be modified.
	auto out = reinterpret_cast<uint8*>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	for (int32 x : Buffer) {
		// Convert the depth value (in millimeters) into a value between 0 - 255. 
		uint8 d = ConvertDepthValueTo8Bit(x, Texture->GetSizeX());
		*out++ = d;
		*out++ = d;
		*out++ = d;
		*out++ = 255;
	}

	Texture->PlatformData->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

// Finds all .OBJ files in the specified Directory, relative to the Content 
// path of the game.
TArray<FString> URealSenseBlueprintLibrary::GetMeshFiles(FString Directory)
{
	// Ensure that the directory ends with a trailing slash
	if (Directory.EndsWith("/") == false) {
		Directory.Append("/");
	}

	// Get the absolute path of the game's Content directory and append the
	// specified path to it, along with the filename.
	FString Dir = FPaths::GameContentDir() + Directory + TEXT("*.obj");

	IFileManager& FileManager = IFileManager::Get();
	TArray<FString> MeshFiles;
	FileManager.FindFiles(MeshFiles, *Dir, true, false);

	return MeshFiles;
}
