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
#include "RealSenseComponent.h"

// Specifies that this component should be initialized and can tick.
URealSenseComponent::URealSenseComponent(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit)
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;

	CameraModel = ECameraModel::None;
	CameraFirmware = "0.0.0.0";

	ColorHorizontalFOV = 0.0f;
	ColorVerticalFOV = 0.0f;
	DepthHorizontalFOV = 0.0f;
	DepthVerticalFOV = 0.0f;

	globalRealSenseSession = nullptr;
}

// When initialized, this component will check if a RealSenseSessionManager actor 
// exists in the scene. If the actor exists, this component stores a reference to 
// it. If it does not, a new RealSenseSessionManager actor will be spawned, and a 
// reference to it will be saved.
void URealSenseComponent::InitializeComponent() 
{
	if (globalRealSenseSession == nullptr) {
		for (TActorIterator<ARealSenseSessionManager> Itr(GetWorld()); Itr; ++Itr) {
			globalRealSenseSession = (ARealSenseSessionManager*)*Itr;
		}
		if (globalRealSenseSession == nullptr) {
			RS_LOG(Log, "Creating RealSenseSessionManager Actor")
			globalRealSenseSession = GetWorld()->SpawnActor<ARealSenseSessionManager>(ARealSenseSessionManager::StaticClass());
		}
	}

	if (globalRealSenseSession)
	{
		globalRealSenseSession->EnableFeature(m_feature);
	}
}

// Queries the camera model, firmware, and field of view data from the RealSense 
// camera.
void URealSenseComponent::BeginPlay() 
{
	CameraModel = globalRealSenseSession->GetCameraModel();
	CameraFirmware = globalRealSenseSession->GetCameraFirmware();

	ColorHorizontalFOV = globalRealSenseSession->GetColorHorizontalFOV();
	ColorVerticalFOV = globalRealSenseSession->GetColorVerticalFOV();

	DepthHorizontalFOV = globalRealSenseSession->GetDepthHorizontalFOV();
	DepthVerticalFOV = globalRealSenseSession->GetDepthVerticalFOV();
}

void URealSenseComponent::EnableFeature()
{
	globalRealSenseSession->EnableFeature(m_feature);
}

void URealSenseComponent::DisableFeature()
{
	globalRealSenseSession->DisableFeature(m_feature);
}

void URealSenseComponent::StartCamera()
{
	globalRealSenseSession->StartCamera();
}

void URealSenseComponent::StopCamera()
{
	globalRealSenseSession->StopCamera();
}

bool URealSenseComponent::IsCameraRunning()
{
	return globalRealSenseSession->IsCameraRunning();
}

FStreamResolution URealSenseComponent::GetColorCameraResolution() 
{
	return globalRealSenseSession->GetColorCameraResolution();
}

void URealSenseComponent::SetColorCameraResolution(EColorResolution resolution)
{
	if (resolution != EColorResolution::UNDEFINED) {
		globalRealSenseSession->SetColorCameraResolution(resolution);
	}
}

FStreamResolution URealSenseComponent::GetDepthCameraResolution()
{
	return globalRealSenseSession->GetDepthCameraResolution();
}

void URealSenseComponent::SetDepthCameraResolution(EDepthResolution resolution) 
{
	if (resolution != EDepthResolution::UNDEFINED) {
		globalRealSenseSession->SetDepthCameraResolution(resolution);
	}
}

bool URealSenseComponent::IsStreamSetValid(EColorResolution ColorResolution, 
										   EDepthResolution DepthResolution) 
{
	return globalRealSenseSession->IsStreamSetValid(ColorResolution, DepthResolution);
}
