#include "RealSensePluginPrivatePCH.h"
#include "ScenePerceptionComponent.h"

UScenePerceptionComponent::UScenePerceptionComponent(const class FObjectInitializer& ObjInit) : Super(ObjInit)
{
//	bWantsInitializeComponent = true;
//	PrimaryComponentTick.bCanEverTick = true;

//	globalRealSenseSession = nullptr;
//	trackingFailed = false;
}

void UScenePerceptionComponent::InitializeComponent() 
{
//	if (globalRealSenseSession == nullptr) {
//		for (TActorIterator<ARealSenseSessionManager> Itr(GetWorld()); Itr; ++Itr) {
//			globalRealSenseSession = (ARealSenseSessionManager*)*Itr;
//		}
//		if (globalRealSenseSession == nullptr)
//			globalRealSenseSession = GetWorld()->SpawnActor<ARealSenseSessionManager>(ARealSenseSessionManager::StaticClass());
//	}
}

void UScenePerceptionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
//	if (!globalRealSenseSession->IsRealSenseEnabled())
//		return;

//	IsCameraConnected = globalRealSenseSession->IsCameraConnected();

//	ColorTexture = globalRealSenseSession->GetColorTexture();
//	DepthTexture = globalRealSenseSession->GetDepthTexture();
//	ReconstructionTexture = globalRealSenseSession->GetReconstructionTexture();
//	PlaneDetectionTexture = globalRealSenseSession->GetPlaneDetectionTexture();

//	SceneQuality = globalRealSenseSession->GetSceneQuality();
//	TrackingAccuracy = globalRealSenseSession->GetTrackingAccuracy();

//	Planes = globalRealSenseSession->GetPlanes();
//	TrackedCameraPose = globalRealSenseSession->GetTrackedCameraPose();
//	globalRealSenseSession->UpdateVertexBuffer(&VertexBuffer, &NormalBuffer);

//	HorizontalFOV = globalRealSenseSession->GetHorizontalFOV();
//	VerticalFOV = globalRealSenseSession->GetVerticalFOV();

//	if ((TrackingAccuracy > ETrackingAccuracy::MEDIUM) && !trackingFailed) {
//		OnTrackingFailure.Broadcast();
//		trackingFailed = true;
//	}
//	else if ((TrackingAccuracy <= ETrackingAccuracy::MEDIUM) && trackingFailed) {
//		OnTrackingRegained.Broadcast();
//		trackingFailed = false;
//	}
}
/*
void UScenePerceptionComponent::EnableRealSense()
{
	globalRealSenseSession->EnableRealSense();
}

void UScenePerceptionComponent::DisableRealSense()
{
	globalRealSenseSession->DisableRealSense();
}

void UScenePerceptionComponent::ResetReconstruction()
{
	globalRealSenseSession->ResetReconstruction();
}

URuntimeMesh* UScenePerceptionComponent::GetSceneMesh()
{
	return globalRealSenseSession->GetSceneMesh();
}

URuntimeMesh* UScenePerceptionComponent::LoadSceneMeshFromFile(int32 num)
{
	return globalRealSenseSession->LoadSceneMeshFromFile(num);
}

void UScenePerceptionComponent::SaveSceneMesh(int32 num)
{
	globalRealSenseSession->SaveSceneMesh(num);
}

FTransform UScenePerceptionComponent::GetMeshOrientationTransform(bool fromFile)
{
	return globalRealSenseSession->GetMeshOrientationTransform(fromFile);
}

URuntimeMesh* UScenePerceptionComponent::RemoveSmallClusters(URuntimeMesh* mesh, int32 threshold) { 
	return globalRealSenseSession->RemoveSmallClusters(mesh, threshold); 
}
*/