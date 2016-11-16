#pragma once

#include "RealSenseTypes.generated.h"

// List of features provided by the RealSense SDK
enum RealSenseFeature : uint8 {
	CAMERA_STREAMING = 0x1,
	SCAN_3D = 0x2,
	HEAD_TRACKING = 0x4,
	SEGMENTATION_3D = 0x8,
	HAND_CURSOR = 0x16,
};

// Resolutions supported by the RealSense RGB camera
UENUM(BlueprintType) 
enum class EColorResolution : uint8 {
	UNDEFINED = 0 UMETA(DisplayName = " "),
	RES1 = 1 UMETA(DisplayName = "1920 x 1080 x 30"),
	RES2 = 2 UMETA(DisplayName = "1280 x 720 x 30"),
	RES3 = 3 UMETA(DisplayName = "640 x 480 x 60"),
	RES4 = 4 UMETA(DisplayName = "640 x 480 x 30"),
	RES5 = 5 UMETA(DisplayName = "320 x 240 x 60"),
	RES6 = 6 UMETA(DisplayName = "320 x 240 x 30"),
};

// Resolutions supported by the RealSense depth camera
// (F200) denotes that this resolution is only supported by the F200 camera.
// (R200) denotes that this resolution is only supported by the R200 camera.
UENUM(BlueprintType) 
enum class EDepthResolution : uint8 {
	UNDEFINED = 0 UMETA(DisplayName = " "),
	RES1 = 1 UMETA(DisplayName = "640 x 480 x 60 (F200)"),
	RES2 = 2 UMETA(DisplayName = "640 x 480 x 30 (F200)"),
	RES3 = 3 UMETA(DisplayName = "628 x 468 x 90 (R200)"),
	RES4 = 4 UMETA(DisplayName = "628 x 468 x 60 (R200)"),
	RES5 = 5 UMETA(DisplayName = "628 x 468 x 30 (R200)"),
	RES6 = 6 UMETA(DisplayName = "480 x 360 x 90 (R200)"),
	RES7 = 7 UMETA(DisplayName = "480 x 360 x 60 (R200)"),
	RES8 = 8 UMETA(DisplayName = "480 x 360 x 30 (R200)"),
	RES9 = 9 UMETA(DisplayName = "320 x 240 x 90 (R200)"),
	RES10 = 10 UMETA(DisplayName = "320 x 240 x 60 (R200)"),
	RES11 = 11 UMETA(DisplayName = "320 x 240 x 30 (R200)"),
};

// Resolutions supported by the RealSense SR300 camera in 3DSeg mode
UENUM(BlueprintType)
enum class E3DSegResolution : uint8 {
	UNDEFINED = 0 UMETA(DisplayName = " "),
	RES1 = 1 UMETA(DisplayName = "color:  640 x 360, depth: 640 x 480"),
	RES2 = 2 UMETA(DisplayName = "color:  960 x 540, depth: 640 x 480"),
	RES3 = 3 UMETA(DisplayName = "color: 1280 x 720, depth: 640 x 480"),
	RES4 = 4 UMETA(DisplayName = "color:  320 x 240, depth: 640 x 480"),
	RES5 = 5 UMETA(DisplayName = "color:  640 x 480, depth: 640 x 480"),
};

// RSSDK Pixel Format exposed to Blueprint (see pxcimage.h)
UENUM(BlueprintType) 
enum class ERealSensePixelFormat : uint8 {
	PIXEL_FORMAT_ANY = 0,  // Unknown/undefined
	COLOR_RGB32,    // BGRA layout
	DEPTH_G16_MM,   // 16-bit unsigned integer with precision mm.
};

// Supported RealSense camera models
UENUM(BlueprintType) 
enum class ECameraModel : uint8 {
	None = 0 UMETA(DisplayName = " "),
	F200 = 1 UMETA(DisplayName = "Front-Facing (F200)"),
	SR300 = 2 UMETA(DisplayName = "Short-Range (SR300)"),
	R200 = 3 UMETA(DisplayName = "World-Facing (R200)"),
	Other = 4 UMETA(DisplayName = "Unknown Camera Model")
};

// Supported modes for the 3D Scanning middleware
UENUM(BlueprintType) 
enum class EScan3DMode : uint8 {
	OBJECT = 0 UMETA(DisplayName = "Object"),
	FACE = 1 UMETA(DisplayName = "Face")
};

// File types supported by the 3D Scanning middleware for saving scans 
UENUM(BlueprintType) 
enum class EScan3DFileFormat : uint8 {
	OBJ = 0 UMETA(DisplayName = "OBJ")
};

// Basic 32-bit color structure (RGBA) 
USTRUCT(BlueprintType) 
struct FSimpleColor
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 R;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 G;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 B;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 A;
};

// Resolution of a RealSense camera stream
USTRUCT(BlueprintType) 
struct FStreamResolution
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 height;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float fps;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERealSensePixelFormat format;
};

// The BodySideType enumerator itemizes the side of a hand cursor
UENUM(BlueprintType)
enum class EBodySideType : uint8 {
	BODY_SIDE_UNKNOWN = 0 UMETA(DisplayName = "The body side is unknown"),
	BODY_SIDE_LEFT = 1 UMETA(DisplayName = "The hand cursor is from a left hand"),
	BODY_SIDE_RIGHT = 2 UMETA(DisplayName = "The hand cursor is from a right hand"),
};

// The AlertType enumerator itemizes the alert events
UENUM(BlueprintType)
enum class EAlertType : uint8 {
	CURSOR_DETECTED = 1 UMETA(DisplayName = "A cursor is detected"),
	CURSOR_NOT_DETECTED = 2 UMETA(DisplayName = "A previous detected cursor is lost"),
	CURSOR_INSIDE_BORDERS = 3 UMETA(DisplayName = "The cursor is moved back to the tracking area"),
	CURSOR_OUT_OF_BORDERS = 4 UMETA(DisplayName = "The cursor is moved out of the tracking area"),
	CURSOR_TOO_CLOSE = 5 UMETA(DisplayName = "The cursor is too close to the camera"),
	CURSOR_TOO_FAR = 6 UMETA(DisplayName = "The cursor is too far away from the camera"),
	CURSOR_OUT_OF_LEFT_BORDER = 7 UMETA(DisplayName = "The cursor is touching the left tracking boundary"),
	CURSOR_OUT_OF_RIGHT_BORDER = 8 UMETA(DisplayName = "The cursor is touching the right tracking boundary"),
	CURSOR_OUT_OF_TOP_BORDER = 9 UMETA(DisplayName = "The cursor is touching the top tracking boundary"),
	CURSOR_OUT_OF_BOTTOM_BORDER = 10 UMETA(DisplayName = "The cursor is touching the bottom tracking boundary"),
	CURSOR_ENGAGED = 11 UMETA(DisplayName = "The cursor is ready to engage with the application"),
	CURSOR_DISENGAGED = 12 UMETA(DisplayName = "The cursor has left the screen bounds or disengaged"),
};

// The Status enumerator itemizes status codes returned by SDK functions
UENUM(BlueprintType)
enum class EStatus : uint8 {
	/* success */
	PXC_STATUS_NO_ERROR = 0 UMETA(DisplayName = "The operation completed successfully"),

	/* errors */
	PXC_STATUS_FEATURE_UNSUPPORTED = 1 UMETA(DisplayName = "The requested feature is not available or not implemented"),
	PXC_STATUS_PARAM_UNSUPPORTED = 2 UMETA(DisplayName = "There are invalid/unsupported parameters in the configuration"),
	PXC_STATUS_ITEM_UNAVAILABLE = 3 UMETA(DisplayName = "The item could not be found, or end of stream"),

	PXC_STATUS_HANDLE_INVALID = 11 UMETA(DisplayName = "The session/module instance or pointer is invalid"),
	PXC_STATUS_ALLOC_FAILED = 12 UMETA(DisplayName = "Failed to allocate memory or create an instance of a module or resource"),

	PXC_STATUS_DEVICE_FAILED = 21 UMETA(DisplayName = "Unexpected device failure due to device malfunctioning"),
	PXC_STATUS_DEVICE_LOST = 22 UMETA(DisplayName = "Unexpected device failure due to the loss of the device"),
	PXC_STATUS_DEVICE_BUSY = 23 UMETA(DisplayName = "The application does not have control of the device"),

	PXC_STATUS_EXEC_ABORTED = 31 UMETA(DisplayName = "The asynchronous pipeline operation is aborted in upstream components"),
	PXC_STATUS_EXEC_INPROGRESS = 32 UMETA(DisplayName = "The asynchronous operation is in progress"),
	PXC_STATUS_EXEC_TIMEOUT = 33 UMETA(DisplayName = "The synchronization function timed out"),

	PXC_STATUS_FILE_WRITE_FAILED = 41 UMETA(DisplayName = "Failed to open a file for write or write to a file"),
	PXC_STATUS_FILE_READ_FAILED = 42 UMETA(DisplayName = "Failed to open a file for read or read a file"),
	PXC_STATUS_FILE_CLOSE_FAILED = 43 UMETA(DisplayName = "Failed to close a file handle"),

	PXC_STATUS_DATA_UNAVAILABLE = 51 UMETA(DisplayName = "Data not available for module processing"),
	PXC_STATUS_DATA_NOT_INITIALIZED = 52 UMETA(DisplayName = "Failed to initialize data"),
	PXC_STATUS_INIT_FAILED = 53 UMETA(DisplayName = "General failure during algorithm initialization"),

	PXC_STATUS_STREAM_CONFIG_CHANGED = 61 UMETA(DisplayName = "The stream configuration has changed during streaming"),

	PXC_STATUS_POWER_UID_ALREADY_REGISTERED = 71 UMETA(DisplayName = "Double registering the same module identifier"),
	PXC_STATUS_POWER_UID_NOT_REGISTERED = 72 UMETA(DisplayName = "The module identifier is not registered"),
	PXC_STATUS_POWER_ILLEGAL_STATE = 73 UMETA(DisplayName = "The operation is not allowed in this state"),
	PXC_STATUS_POWER_PROVIDER_NOT_EXISTS = 74 UMETA(DisplayName = "The power manager is not available"),

	PXC_STATUS_CAPTURE_CONFIG_ALREADY_SET = 81 UMETA(DisplayName = "The capture configuration has already been set"),
	PXC_STATUS_COORDINATE_SYSTEM_CONFLICT = 82 UMETA(DisplayName = "Mismatched coordinate system definitions"),
	PXC_STATUS_NOT_MATCHING_CALIBRATION = 83 UMETA(DisplayName = "The calibration process failed to match the data points"),

	PXC_STATUS_ACCELERATION_UNAVAILABLE = 91 UMETA(DisplayName = "The hardware accelerator required by the algorithm is not available on this platform"),

	/* warnings */
	PXC_STATUS_TIME_GAP = 101 UMETA(DisplayName = "There is a gap in time stamps"),
	PXC_STATUS_PARAM_INPLACE = 102 UMETA(DisplayName = "The algorithm is already configured with the same parameters"),
	PXC_STATUS_DATA_NOT_CHANGED = 103 UMETA(DisplayName = "Data remains the same as no new input is available"),
	PXC_STATUS_PROCESS_FAILED = 104 UMETA(DisplayName = "General failure in algorithm processing"),
	PXC_STATUS_VALUE_OUT_OF_RANGE = 105 UMETA(DisplayName = "The data value(s) is out of range"),
	PXC_STATUS_DATA_PENDING = 106 UMETA(DisplayName = "There is additional data remaining in the algorithm buffer"),
};
