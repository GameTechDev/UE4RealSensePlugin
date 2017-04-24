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
#pragma once

#include "RealSenseComponent.h"
#include "HeadTrackingComponent.generated.h"

UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = RealSense) 
class UHeadTrackingComponent : public URealSenseComponent
{
	GENERATED_UCLASS_BODY()
		
	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
	int32 HeadCount;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
	FVector HeadPosition;

	UPROPERTY(BlueprintReadOnly, Category = "RealSense")
	FRotator HeadRotation;

	UHeadTrackingComponent();

	void InitializeComponent() override;
	
	void TickComponent(float DeltaTime, enum ELevelTick TickType, 
		               FActorComponentTickFunction *ThisTickFunction) override;

private:
};
