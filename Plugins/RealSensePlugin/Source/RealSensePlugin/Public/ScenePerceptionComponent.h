#pragma once

#include "RealSenseSessionManager.h"
#include "ScenePerceptionComponent.generated.h"

UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = RealSense) class UScenePerceptionComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()
/*
	// Pointer to application-wide RealSense session manager
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") ARealSenseSessionManager* globalRealSenseSession;

	// Boolean indicating whether a RealSense camera is connected to the computer
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") bool IsCameraConnected;

	// Texture displaying the RGB frame of the RealSense camera
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") UTexture2D* ColorTexture;
	// Texture displaying the depth frame of the RealSense camera
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") UTexture2D* DepthTexture;
	// Texture displaying the accumulation of scene data
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") UTexture2D* ReconstructionTexture;
	// Texture displaying the detected planes in the scene
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") UTexture2D* PlaneDetectionTexture;

	// Floating point value between 0.0 - 1.0 indicating how easy it will be to perform tracking on the scene 
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") float SceneQuality;
	// A value of Low, Medium, or High indicating the confidence that the tracked device pose is accurate.
	// If the value is Failed, then RealSense has lost tracking and OnTrackingFailure will be Broadcast.
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") ETrackingAccuracy TrackingAccuracy;

	// Array of FPlanes representing the detected planes in the scene
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") TArray<FPlane> Planes;
	// Transform representing the Location and Rotation of the RealSense camera relative to its starting position. 
	// The Scale is not meaningful in this context and will be (1.0, 1.0, 1.0).
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") FTransform TrackedCameraPose;
	// Array of Vector3s representing the 3D coordinates of the pixels in the RealSense image, downsampled by a factor of 4.
	// The size of this array is 19200 = (320 * 240 / 4). Use in conjunction with NormalBuffer for normal information.
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") TArray<FVector> VertexBuffer;
	// Array of Vector3s representing the normals of the pixels in the RealSense image, downsampled by a factor of 4.
	// The size of this array is 19200 = (320 * 240 / 4). Use in conjunction with VertexBuffer for the 3D coordinate of each pixel.
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") TArray<FVector> NormalBuffer;

	// Horizontal Field of View of the RealSense camera
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") float HorizontalFOV;
	// Vertical Field of View of the RealSense camera
	UPROPERTY(BlueprintReadOnly, Category = "RealSense") float VerticalFOV;
	
	// This event is triggered when RealSense tracking fails. Use this in conjunction with 
	// OnTrackingRegained to pause tracking-related functionality.
	UPROPERTY(BlueprintAssignable, Category = "RealSense") FRealSenseNullaryDelegate OnTrackingFailure;
	// This event is triggered when RealSense tracking is regained. Use this in conjunction with 
	// OnTrackingFailed to pause tracking-related functionality.
	UPROPERTY(BlueprintAssignable, Category = "RealSense") FRealSenseNullaryDelegate OnTrackingRegained;

	// Enable RealSense camera functionality.
	UFUNCTION(BlueprintCallable, Category = "RealSense") void EnableRealSense();
	// Disable RealSense camera functionality.
	UFUNCTION(BlueprintCallable, Category = "RealSense") void DisableRealSense();

	// Reset tracking and accumulation of scene reconstruction data. 
	UFUNCTION(BlueprintCallable, Category = "RealSense") void ResetReconstruction();

	// Returns a RuntimeMesh representing the scene of the current RealSense session.
	// This can be used to set the Mesh attribute of a RuntimeMeshComponent during gameplay.
	UFUNCTION(BlueprintCallable, Category = "RealSense") URuntimeMesh* GetSceneMesh();
	// Returns a RuntimeMesh from a previously saved .obj file created by calling SaveSceneMesh().
	// The name of the .obj file that will be loaded is "mesh"<num>".obj".
	// This can be used to set the Mesh attribute of a RuntimeMeshComponent during gameplay.
	UFUNCTION(BlueprintCallable, Category = "RealSense") URuntimeMesh* LoadSceneMeshFromFile(int32 num);
	// Save the accumulated scene of the current RealSense session to an .obj file named "mesh"<num>".obj".
	UFUNCTION(BlueprintCallable, Category = "RealSense") void SaveSceneMesh(int32 num);
	// Returns a transform which can be used to orient the RealSense scene mesh. Internally, this function
	// uses the normal vector of the largest detected plane to represent the Z-UP direction. If plane detection
	// is disabled, the transform will be an identity matrix. 
	// Use the fromFile flag if the scene mesh was loaded from a .obj file.
	UFUNCTION(BlueprintCallable, Category = "RealSense") FTransform GetMeshOrientationTransform(bool fromFile);

	UFUNCTION(BlueprintCallable, Category = "RealSense") URuntimeMesh* RemoveSmallClusters(URuntimeMesh* mesh, int32 threshold);
*/
	UScenePerceptionComponent();

	void InitializeComponent() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

//private:
//	bool trackingFailed;
};
