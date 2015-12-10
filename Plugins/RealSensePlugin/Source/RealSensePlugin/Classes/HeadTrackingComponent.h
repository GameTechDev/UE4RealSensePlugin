#pragma once

#include "RealSenseComponent.h"
#include "RealSenseSessionManager.h"
#include "RealSenseTypes.h"
#include "HeadTrackingComponent.generated.h"

// This component provides access to a buffer of RGB camera data and 
// a buffer of depth camera data, as well as convenient Texture objects
// for displaying this data.
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = RealSense) class UHeadTrackingComponent : public URealSenseComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RealSense") int32 HeadCount;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense") FVector HeadPosition;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense") FRotator HeadRotation;

	// Default Constructor
	UHeadTrackingComponent();

	// Adds the CAMERA_STREAMING feature to the RealSenseSessionManager and
	// initializes the ColorTexture and DepthTexture objects.
	void InitializeComponent() override;

	// Copies the ColorBuffer and DepthBuffer from the RealSenseSessionManager.
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
