#pragma once

#include "RealSenseComponent.h"
#include "HandCursorComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHandCursorDataDelegate, FVector, Data);


UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = RealSense) 
class UHandCursorComponent : public URealSenseComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
	FVector HandCursorData;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
	bool IsHandCursorDataOk;

	UPROPERTY(BlueprintAssignable, Category = "RealSense")
	FHandCursorDataDelegate OnHandCursorData;

	UHandCursorComponent();

	void InitializeComponent() override;
	
	void TickComponent(float DeltaTime, enum ELevelTick TickType, 
		               FActorComponentTickFunction *ThisTickFunction) override;
};
