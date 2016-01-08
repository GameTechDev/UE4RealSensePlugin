#pragma once

#include "RealSenseTypes.h"
#include "pxc3dscan.h"

// Log Category that can be used by all RealSensePlugin source files that inclue this file
DECLARE_LOG_CATEGORY_EXTERN(RealSensePlugin, Log, All);

#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

// Modified macro from tutorial by Spoof and Kris: https://wiki.unrealengine.com/Log_Macro_with_Netmode_and_Colour
#define RS_LOG(Verbosity, Format, ...) \
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
    UE_LOG(RealSensePlugin, Verbosity, TEXT("%s() : %s"), TEXT(__func__), *Msg); \
}

// Helper log macro for logging pxcStatus codes
#define RS_LOG_STATUS(Status, Format, ...) \
{ \
    const FString Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__); \
	const int Stat = static_cast<int>(Status); \
	if (Stat < 0) \
	    UE_LOG(RealSensePlugin, Error, TEXT("%s() : %s : %d"), TEXT(__func__), *Msg, Stat) \
		else if (Stat == 0) \
		UE_LOG(RealSensePlugin, Log, TEXT("%s() : %s : %d"), TEXT(__func__), *Msg, Stat) \
		else if (Stat > 0) \
		UE_LOG(RealSensePlugin, Warning, TEXT("%s() : %s : %d"), TEXT(__func__), *Msg, Stat) \
}

// Converts a Vector from RealSense camera space to UE4 world space.
FVector ConvertRSVectorToUnreal(FVector v);

// Converts a depth value (in millimeters) to an 8-bit scale (between 0 - 255).
uint8 ConvertDepthValueTo8Bit(int32 depth, int32 width);

// Returns a StreamResolution structure containing the values from the enumerated ColorResolution
FStreamResolution GetEColorResolutionValue(EColorResolution res);

// Returns a StreamResolution structure containing the values from the enumerated DepthResolution
FStreamResolution GetEDepthResolutionValue(EDepthResolution res);

// Converts a Blueprint-exposed RealSensePixelFormat to a PXCImage::PixelFormat
PXCImage::PixelFormat GetPXCPixelFormat(ERealSensePixelFormat format);

// Converts a Blueprint-exposed RealSensePixelFormat to a PXCImage::PixelFormat
PXC3DScan::ScanningMode GetPXCScanningMode(EScan3DMode mode);

// Converts a Blueprint-exposed RealSensePixelFormat to a PXCImage::PixelFormat
PXC3DScan::FileFormat GetPXCScanFileFormat(EScan3DFileFormat format);

// Copies the data from the input color PXCImage into the input data structure.
void CopyColorImageToBuffer(PXCImage* image, TArray<uint8>& data, const uint32 width, const uint32 height);

// Copies the data from the input depth PXCImage into the input data structure.
void CopyDepthImageToBuffer(PXCImage* image, TArray<uint16>& data, const uint32 width, const uint32 height);

void LoadMeshFile(const FString& filename, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FColor>& Colors);
