#pragma once

#include "RealSenseComponent.h"
#include "BlobTrackingComponent.generated.h"

UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = RealSense) 
class UBlobTrackingComponent : public URealSenseComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
		int32 BlobCount;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
		FVector BlobCenter;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
		FVector BlobClosest;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
		FVector BlobTop;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
		FVector BlobBottom;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
		FVector BlobLeft;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
		FVector BlobRight;

	UBlobTrackingComponent();

	void InitializeComponent() override;
	
	void TickComponent(float DeltaTime, enum ELevelTick TickType, 
		               FActorComponentTickFunction *ThisTickFunction) override;

private:
};
