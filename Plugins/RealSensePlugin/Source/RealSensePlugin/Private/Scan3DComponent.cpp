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
#include "Scan3DComponent.h"

UScan3DComponent::UScan3DComponent(const class FObjectInitializer& ObjInit) 
	: Super(ObjInit) 
{ 
	bHasScanStarted = false;
	m_feature = RealSenseFeature::SCAN_3D;
}

// Adds the SCAN_3D feature to the RealSenseSessionManager and initializes the 
// ScanTexture object.
void UScan3DComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ScanTexture = UTexture2D::CreateTransient(1, 1,	EPixelFormat::PF_B8G8R8A8);
}

// Copies the ScanBuffer and checks if a current scan has just completed.
// If it has, the OnScanComplete event is broadcast.
void UScan3DComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, 
	                                 FActorComponentTickFunction *ThisTickFunction) 
{
	if (globalRealSenseSession->IsCameraRunning() == false) {
		return;
	}

	// The 3D Scanning preview image size can be changed automatically by the
	// middleware, so it is important to check every tick if the image size
	// has changed so that the ScanTexture object can be resized to match.
	if (globalRealSenseSession->HasScan3DImageSizeChanged()) {
		int Scan3DImageWidth = globalRealSenseSession->GetScan3DImageWidth();
		int Scan3DImageHeight = globalRealSenseSession->GetScan3DImageHeight();
		ScanTexture = UTexture2D::CreateTransient(Scan3DImageWidth, Scan3DImageHeight,
											      EPixelFormat::PF_B8G8R8A8);
		ScanTexture->UpdateResource();
	}

	ScanBuffer = globalRealSenseSession->GetScanBuffer();

	if (globalRealSenseSession->HasScanCompleted() && bHasScanStarted) {
		OnScanComplete.Broadcast();
		bHasScanStarted = false;
	}
}

void UScan3DComponent::ConfigureScanning(EScan3DMode ScanningMode, bool bSolidify)
{
	globalRealSenseSession->ConfigureScanning(ScanningMode, bSolidify, false);
}

void UScan3DComponent::StartScanning()
{
	globalRealSenseSession->StartScanning();
	bHasScanStarted = true;
}

void UScan3DComponent::StopScanning()
{
	globalRealSenseSession->StopScanning();
}

void UScan3DComponent::SaveScan(FString Filename)
{
	Filename = FPaths::GameContentDir().Append(Filename);
	globalRealSenseSession->SaveScan(EScan3DFileFormat::OBJ, Filename);
}

void UScan3DComponent::LoadScan(FString Filename)
{
	Filename = FPaths::GameContentDir().Append(Filename);
	LoadMeshFile(Filename, Vertices, Triangles, Colors);
}

bool UScan3DComponent::IsScanning() 
{
	return globalRealSenseSession->IsScanning();
}
