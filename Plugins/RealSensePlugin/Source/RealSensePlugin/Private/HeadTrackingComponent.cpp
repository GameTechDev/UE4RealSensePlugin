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
#include "RealSensePluginPrivatePCH.h"
#include "HeadTrackingComponent.h"

UHeadTrackingComponent::UHeadTrackingComponent(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit) 
{ 
	m_feature = RealSenseFeature::HEAD_TRACKING;
}

void UHeadTrackingComponent::InitializeComponent()
{
	Super::InitializeComponent();

	HeadCount = 0;
	HeadPosition = FVector(0.0f, 0.0f, 0.0f);
	HeadRotation = FRotator(0.0f, 0.0f, 0.0f);
}

void UHeadTrackingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, 
	                                 FActorComponentTickFunction *ThisTickFunction) 
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		return;
	}

	HeadCount = globalRealSenseSession->GetHeadCount();
	HeadPosition = globalRealSenseSession->GetHeadPosition();
	HeadRotation = globalRealSenseSession->GetHeadRotation();
}
