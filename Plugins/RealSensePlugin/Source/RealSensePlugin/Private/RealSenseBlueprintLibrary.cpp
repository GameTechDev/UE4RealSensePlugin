#include "RealSensePluginPrivatePCH.h"
#include "RealSenseBlueprintLibrary.h"

URealSenseBlueprintLibrary::URealSenseBlueprintLibrary(const class FObjectInitializer& ObjInit) : Super(ObjInit) 
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
	case ECameraModel::Other:
		return FString("Unknown Camera Model");
	default:
		return FString(" ");
	}
}

// Copies the data from the input Buffer into the PlatformData of the Texture object.
// For convenience, this function returns a pointer to the input Texture that was modified.
UTexture2D* URealSenseBlueprintLibrary::ColorBufferToTexture(const TArray<FSimpleColor>& Buffer, UTexture2D* Texture) 
{
	if (Texture == nullptr)
		return nullptr;

	if (Buffer.Num() != Texture->GetSizeX() * Texture->GetSizeY()) {
		RS_LOG(Error, "Buffer / Texture Size Mismatch")
		return nullptr;
	}
	
	// The Texture's PlatformData needs to be locked before it can be modified.
	auto out = reinterpret_cast<uint8_t *>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	int size = Texture->GetSizeX() * Texture->GetSizeY() * 4;
	memcpy_s(out, size, Buffer.GetData(), size);

	Texture->PlatformData->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

// Copies the data from the input Buffer into the PlatformData of the Texture object.
// For convenience, this function returns a pointer to the input Texture that was modified.
UTexture2D* URealSenseBlueprintLibrary::DepthBufferToTexture(const TArray<int32>& Buffer, UTexture2D* Texture)
{
	if (Texture == nullptr)
		return nullptr;

	if (Buffer.Num() != Texture->GetSizeX() * Texture->GetSizeY()) {
		RS_LOG(Error, "Buffer / Texture Size Mismatch")
		return nullptr;
	}

	// The Texture's PlatformData needs to be locked before it can be modified.
	auto out = reinterpret_cast<uint8_t *>(Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	for (int x : Buffer) {
		// Convert the depth value (in millimeters) into a value between 0 - 255. 
		uint8_t d = ConvertDepthValueTo8Bit(x, Texture->GetSizeX());
		*out++ = d;
		*out++ = d;
		*out++ = d;
		*out++ = 255;
	}

	Texture->PlatformData->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

TArray<FString> URealSenseBlueprintLibrary::GetMeshFiles(FString Directory)
{
	TArray<FString> MeshFiles;
	IFileManager& FileManager = IFileManager::Get();
	FString Dir = FPaths::GameContentDir() + Directory + TEXT("/*.obj");
	FileManager.FindFiles(MeshFiles, *Dir, true, false);
	return MeshFiles;
}