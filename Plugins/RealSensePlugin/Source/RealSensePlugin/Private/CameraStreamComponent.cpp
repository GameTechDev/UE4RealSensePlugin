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
#include "CameraStreamComponent.h"

UCameraStreamComponent::UCameraStreamComponent(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit) 
{ 
	m_feature = RealSenseFeature::CAMERA_STREAMING;
}

// Adds the CAMERA_STREAMING feature to the RealSenseSessionManager and
// initializes the ColorTexture and DepthTexture objects.
void UCameraStreamComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ColorTexture = UTexture2D::CreateTransient(1, 1, EPixelFormat::PF_B8G8R8A8);
	DepthTexture = UTexture2D::CreateTransient(1, 1, EPixelFormat::PF_B8G8R8A8);
}

// Copies the ColorBuffer and DepthBuffer from the RealSenseSessionManager.
void UCameraStreamComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, 
	                                       FActorComponentTickFunction *ThisTickFunction)
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		return;
	}

	ColorBuffer = globalRealSenseSession->GetColorBuffer();
	DepthBuffer = globalRealSenseSession->GetDepthBuffer();
}

// If the supplied resolution is valid, this function will pass that resolution
// along to the RealSenseSessionManager and recreate the ColorTexture objects
// to have the same resolution.
void UCameraStreamComponent::SetColorCameraResolution(EColorResolution resolution) 
{
	if (resolution == EColorResolution::UNDEFINED) {
		return;
	}

	Super::SetColorCameraResolution(resolution);

	int ColorImageWidth = globalRealSenseSession->GetColorImageWidth();
	int ColorImageHeight = globalRealSenseSession->GetColorImageHeight();
	ColorTexture = UTexture2D::CreateTransient(ColorImageWidth, ColorImageHeight,
											   PF_B8G8R8A8);
	ColorTexture->UpdateResource();
}

// If the supplied resolution is valid, this function will pass that resolution
// along to the RealSenseSessionManager and recreate the DepthTexture objects
// to have the same resolution.
void UCameraStreamComponent::SetDepthCameraResolution(EDepthResolution resolution)
{
	if (resolution == EDepthResolution::UNDEFINED) {
		return;
	}

	Super::SetDepthCameraResolution(resolution);
	
	int DepthImageWidth = globalRealSenseSession->GetDepthImageWidth();
	int DepthImageHeight = globalRealSenseSession->GetDepthImageHeight();
	DepthTexture = UTexture2D::CreateTransient(DepthImageWidth, DepthImageHeight, 
											   PF_B8G8R8A8);
	DepthTexture->UpdateResource();
}

// Enable 3D segmentation.
void UCameraStreamComponent::Enable3DSegmentation(bool b3DSeg)
{
	if (b3DSeg)
	{
		m_feature = RealSenseFeature::SEGMENTATION_3D;
		Super::EnableFeature();
	}
}