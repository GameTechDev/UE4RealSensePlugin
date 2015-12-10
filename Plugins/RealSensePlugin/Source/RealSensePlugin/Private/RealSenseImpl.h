#pragma once

#include "AllowWindowsPlatformTypes.h"
#include <future>
#include <iostream>
#include <fstream>
#include "HideWindowsPlatformTypes.h"

#include "CoreMisc.h"

#include "RealSenseTypes.h"
#include "RealSenseUtils.h"
#include "PXCSenseManager.h"

// Stores all relevant data computed from one frame of RealSense camera data.
// Advice: Use this structure in a multiple-buffer configuration to share 
// RealSense data between threads.
// Example usage: 
//   Thread 1: Process camera data and populate background_frame
//             Swap background_frame with mid_frame
//   Thread 2: Swap mid_frame with foreground_frame
//             Read data from foreground_frame
struct RealSenseDataFrame {
	uint64 number;  // Stores an ID for the frame based on its occurrence in time
	TArray<uint8> colorImage;  // Container for the camera's raw color stream data
	TArray<uint16> depthImage;  // Container for the camera's raw depth stream data
	TArray<uint8> scanImage;  // Container for the scan preview image provided by the 3DScan middleware

	RealSenseDataFrame() : number(0) {}
};

// Implements the functionality of the Intel(R) RealSense(TM) SDK and associated
// middleware modules as used by the RealSenseSessionManager Actor class.
//
// NOTE: Some function declarations do not have a function comment because the 
// comment is written in RealSenseSessionManager.h. 
class RealSenseImpl {
public:
	// Creates a new RealSense Session and queries physical device information.
	RealSenseImpl();

	// Terminates the camera processing thread and releases handles to Core SDK objects.
	~RealSenseImpl();

	// Performs RealSense Core and Middleware processing based on the enabled
	// feature set.
	//
	// This function is meant to be run on its own thread to minimize the 
	// performance impact on the game thread.
	void CameraThread();

	void StartCamera();

	void StopCamera();

	// Swaps the data frames to load the latest processed data into the 
	// foreground frame.
	void SwapFrames();

	inline bool IsCameraThreadRunning() const { return cameraThreadRunning.load(); }

	/* 
	 * Core SDK Support 
	 */

	// Enables the middleware specified by the input feature set and creates
	// handles to the necessary RSSDK objects. 
	void EnableRealSenseFeatures(uint32 featureSet);

	inline bool IsCameraConnected() const { return (senseManager->IsConnected() != 0); }

	inline const float GetColorHorizontalFOV() const { return colorHorizontalFOV; }

	inline const float GetColorVerticalFOV() const { return colorVerticalFOV; }

	inline const float GetDepthHorizontalFOV() const { return depthHorizontalFOV; }

	inline const float GetDepthVerticalFOV() const { return depthVerticalFOV; }

	const ECameraModel GetCameraModel() const;

	const FString GetCameraFirmware() const;

	inline FStreamResolution GetColorCameraResolution() const { return colorResolution; }

	inline const int GetColorImageWidth() const { return colorResolution.width; }

	inline const int GetColorImageHeight() const { return colorResolution.height; }

	void SetColorCameraResolution(EColorResolution resolution);

	inline FStreamResolution GetDepthCameraResolution() const { return depthResolution; }

	inline const int GetDepthImageWidth() const { return depthResolution.width; }

	inline const int GetDepthImageHeight() const { return depthResolution.height; }

	void SetDepthCameraResolution(EDepthResolution resolution);

	bool IsStreamSetValid(EColorResolution ColorResolution, EDepthResolution DepthResolution);

	inline const uint8* GetColorBuffer() const { return fgFrame->colorImage.GetData(); }

	inline const uint16* GetDepthBuffer() const { return fgFrame->depthImage.GetData(); }

	/* 
	 * 3D Scanning Module Support 
	 */

	bool ConfigureScanning(EScan3DMode scanningMode, bool solidify, bool texture);

	void SetScanningVolume(FVector boundingBox, int32 resolution);

	void StartScanning();

	void StopScanning();

	void ResetScanning();

	void SaveScan(EScan3DFileFormat saveFileFormat, const FString& filename);
	
	void LoadScan(const FString& filename, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FColor>& Colors);

	inline bool IsScanning() const { return (p3DScan->IsScanning() != 0); }

	inline FStreamResolution GetScan3DResolution() const { return scan3DResolution; }

	inline int GetScan3DImageWidth() const { return scan3DResolution.width; }

	inline int GetScan3DImageHeight() const { return scan3DResolution.height; }

	inline const uint8* GetScanBuffer() const { return fgFrame->scanImage.GetData(); }

	inline bool HasScan3DImageSizeChanged() const { return scan3DImageSizeChanged.load(); }

	inline bool HasScanCompleted() const { return scanCompleted.load(); }

private:
	// Core SDK handles

	struct RealSenseDeleter {
		void operator()(PXCSession* s) { s->Release(); }
		void operator()(PXCSenseManager* sm) { sm->Release(); }
		void operator()(PXCCapture* c) { c->Release(); }
		void operator()(PXCCapture::Device* d) { d->Release(); }
		void operator()(PXC3DScan* sc) { ; }
	};

	std::unique_ptr<PXCSession, RealSenseDeleter> session;
	std::unique_ptr<PXCSenseManager, RealSenseDeleter> senseManager;
	std::unique_ptr<PXCCapture, RealSenseDeleter> capture;
	std::unique_ptr<PXCCapture::Device, RealSenseDeleter> device;

	PXCCapture::DeviceInfo deviceInfo;
	pxcStatus status;  // Status ID used by RSSDK functions

	// SDK Module handles

	std::unique_ptr<PXC3DScan, RealSenseDeleter> p3DScan;

	// Feature set constructed as the logical OR of RealSenseFeatures
	uint32 RealSenseFeatureSet;

	std::atomic_bool colorStreamingEnabled;
	std::atomic_bool depthStreamingEnabled;
	std::atomic_bool scan3DEnabled;

	// Camera processing members

	std::thread cameraThread;
	std::atomic_bool cameraThreadRunning;

	std::unique_ptr<RealSenseDataFrame> fgFrame;
	std::unique_ptr<RealSenseDataFrame> midFrame;
	std::unique_ptr<RealSenseDataFrame> bgFrame;

	// Mutex for locking access to the midFrame
	std::mutex midFrameMutex;

	// Core SDK members

	FStreamResolution colorResolution;
	FStreamResolution depthResolution;

	float colorHorizontalFOV;
	float colorVerticalFOV;
	float depthHorizontalFOV;
	float depthVerticalFOV;

	// 3D Scan members

	FStreamResolution scan3DResolution;

	PXC3DScan::FileFormat scan3DFileFormat;
	FString scan3DFilename;

	std::atomic_bool scanStarted;
	std::atomic_bool scanStopped;
	std::atomic_bool reconstructEnabled;
	std::atomic_bool scanCompleted;
	std::atomic_bool scan3DImageSizeChanged;

	// Helper Functions

	void UpdateScan3DImageSize(PXCImage::ImageInfo info);
};
