#include "RealSensePluginPrivatePCH.h"
#include "RealSenseUtils.h"

DEFINE_LOG_CATEGORY(RealSensePlugin);

// Shuffles the input Vector's coordinates around to convert it
// to Unreal world space.
FVector ConvertRSVectorToUnreal(FVector v)
{
	return FVector(v.Z, v.X, -v.Y);
}

// Maps the depth value to a number between 0 - 255 so it can
// be represented as an 8-bit color.
uint8 ConvertDepthValueTo8Bit(int32 depth, int32 width)
{
	// The F200 and R200 cameras support different maximum depths.
	float max_depth = 0.0f;
	if (width == 640) {
		max_depth = 1000.0f; // 1 meter
	}
	else {
		max_depth = 3000.0f; // 3 meters
	}

	// A depth value of 0 indicates no data available.
	// This value will be mapped to the color black.
	if ((depth == 0) || (depth > max_depth)) {
		return 0;
	}

	return (255 * ((max_depth - depth) / max_depth));
}

PXCImage::PixelFormat GetPXCPixelFormat(ERealSensePixelFormat format)
{
	switch (format) {
	case ERealSensePixelFormat::COLOR_RGB32:
		return PXCImage::PixelFormat::PIXEL_FORMAT_RGB32;
	case ERealSensePixelFormat::DEPTH_G16_MM:
		return PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH;
	default:
		return PXCImage::PixelFormat::PIXEL_FORMAT_ANY;
	}
}

PXC3DScan::ScanningMode GetPXCScanningMode(EScan3DMode mode)
{
	switch (mode) {
	case EScan3DMode::OBJECT:
		return PXC3DScan::ScanningMode::OBJECT_ON_PLANAR_SURFACE_DETECTION;
	case EScan3DMode::FACE:
		return PXC3DScan::ScanningMode::FACE;
	default:
		return PXC3DScan::ScanningMode::FACE;
	}
}

PXC3DScan::FileFormat GetPXCScanFileFormat(EScan3DFileFormat format)
{
	switch (format) {
	case EScan3DFileFormat::OBJ:
		return PXC3DScan::FileFormat::OBJ;
	default:
		return PXC3DScan::FileFormat::OBJ;
	}
}

// Extracts the width, height, and fps values from each enumerated resolution.
// Note: The only color pixel format currently supported is RGB32.
FStreamResolution GetEColorResolutionValue(EColorResolution res) 
{
	FStreamResolution resolution = {};
	switch (res) {
	case EColorResolution::RES1:
		return{ 1920, 1080, 30.0f, ERealSensePixelFormat::COLOR_RGB32 };
	case EColorResolution::RES2:
		return{ 1280, 720, 30.0f, ERealSensePixelFormat::COLOR_RGB32 };
	case EColorResolution::RES3:
		return{ 640, 480, 60.0f, ERealSensePixelFormat::COLOR_RGB32 };
	case EColorResolution::RES4:
		return{ 640, 480, 30.0f, ERealSensePixelFormat::COLOR_RGB32 };
	case EColorResolution::RES5:
		return{ 320, 240, 60.0f, ERealSensePixelFormat::COLOR_RGB32 };
	case EColorResolution::RES6:
		return{ 320, 240, 30.0f, ERealSensePixelFormat::COLOR_RGB32 };
	default:
		return{ 0, 0, 0.0f, ERealSensePixelFormat::PIXEL_FORMAT_ANY };
	}
}

// Extracts the width, height, and fps values from each enumerated resolution.
// Note: The only depth pixel format currently supported is 16-bit grayscale.
FStreamResolution GetEDepthResolutionValue(EDepthResolution res)
{
	FStreamResolution resolution = {};
	switch (res) {
	case EDepthResolution::RES1:
		return{ 640, 480, 60.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES2:
		return{ 640, 480, 30.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES3:
		return{ 628, 468, 90.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES4:
		return{ 628, 468, 60.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES5:
		return{ 628, 468, 30.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES6:
		return{ 480, 360, 90.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES7:
		return{ 480, 360, 60.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES8:
		return{ 480, 360, 30.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES9:
		return{ 320, 240, 90.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES10:
		return{ 320, 240, 60.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	case EDepthResolution::RES11:
		return{ 320, 240, 30.0f, ERealSensePixelFormat::DEPTH_G16_MM };
	default:
		return{ 0, 0, 0.0f, ERealSensePixelFormat::PIXEL_FORMAT_ANY };
	}
}

// Original function borrowed from RSSDK sp_glut_utils.h
// Copies the data from the PXCImage into the input data buffer.
void CopyColorImageToBuffer(PXCImage* image, TArray<uint8>& data, const uint32 width, const uint32 height)
{
	// Extracts the raw data from the PXCImage object.
	PXCImage::ImageData imageData;
	pxcStatus result = image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB24, &imageData);
	if (result != PXC_STATUS_NO_ERROR) {
		return;
	}

	uint32 i = 0;
	for (uint32 y = 0; y < height; ++y) {
		// color points to one row of color image data.
		const pxcBYTE* color = imageData.planes[0] + (imageData.pitches[0] * y);
		for (uint32 x = 0; x < width; ++x, color += 3) {
			data[i++] = color[0];
			data[i++] = color[1];
			data[i++] = color[2];
			data[i++] = 0xff; // alpha = 255
		}
	}
	
	image->ReleaseAccess(&imageData);
}

// Original function borrowed from RSSDK sp_glut_utils.h
// Copies the data from the PXCImage into the input data buffer.
void CopyDepthImageToBuffer(PXCImage* image, TArray<uint16>& data, const uint32 width, const uint32 height)
{
	// Extracts the raw data from the PXCImage object.
	PXCImage::ImageData imageData;
	pxcStatus result = image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &imageData);
	if (result != PXC_STATUS_NO_ERROR)
		return;

	const uint32 numBytes = width * sizeof(uint16);

	uint32 i = 0;
	for (uint32 y = 0; y < height; ++y) {
		// depth points to one row of depth image data.
		const pxcBYTE* depth = imageData.planes[0] + (imageData.pitches[0] * y);
		for (uint32 x = 0; x < width; ++x, depth += 2) {
			data[i++] = *depth;
		}
	}

	image->ReleaseAccess(&imageData);
}

void LoadMeshFile(const FString& filename, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FColor>& Colors)
{
	// TODO: Check if Reserving Lines ahead of time is faster
	TArray<FString> Lines;
	if (FFileHelper::LoadANSITextFileToStrings(filename.GetCharArray().GetData(), NULL, Lines) == false)
		return;

	Vertices.Empty();
	Triangles.Empty();
	Colors.Empty();

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	int v1 = 0;
	int v2 = 0;
	int v3 = 0;

	TArray<FString> Tokens;

	for (FString Line : Lines) {
		if (Line.IsEmpty()) {
			continue;
		}
		else if (Line[0] == 'v') {
			if (Line[1] == ' ') {
				Tokens.Empty();
				Line.ParseIntoArrayWS(Tokens, L"", true);
				x = FCString::Atof(*(Tokens[1]));
				y = FCString::Atof(*(Tokens[2]));
				z = FCString::Atof(*(Tokens[3]));
				r = FCString::Atof(*(Tokens[4]));
				g = FCString::Atof(*(Tokens[5]));
				b = FCString::Atof(*(Tokens[6]));
				Vertices.Add(ConvertRSVectorToUnreal(FVector(x, y, z)) * 150);
				Colors.Add(FColor((uint8)(r * 255), (uint8)(g * 255), (uint8)(b * 255)));
			}
		}
		else if (Line[0] == 'f') {
			Tokens.Empty();
			Line.ParseIntoArrayWS(Tokens, L"//", true);
			// Need to subtract 1 from the vertex indices because .OBJ files start indexing them at at 1, not 0
			v1 = FCString::Atoi(*(Tokens[1]));
			v2 = FCString::Atoi(*(Tokens[3]));
			v3 = FCString::Atoi(*(Tokens[5]));
			Triangles.Add(v1 - 1);
			Triangles.Add(v2 - 1);
			Triangles.Add(v3 - 1);
		}
	}

	FVector MeshCenter = FVector(0.0f, 0.0f, 0.0f);
	for (FVector Vert : Vertices) {
		MeshCenter += Vert;
	}

	MeshCenter /= Vertices.Num();

	for (int i = 0; i < Vertices.Num(); i++) {
		Vertices[i] -= MeshCenter;
	}
}
