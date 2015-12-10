// This class is intentionally left empty until the Procedural Mesh Component is availabled in C++. 
// Currently, this feature is experimental and only available in Blueprints.

#include "RealSensePluginPrivatePCH.h"
#include "ProceduralMeshActor.h"

AProceduralMeshActor::AProceduralMeshActor(const class FObjectInitializer& Init) : Super(Init) 
{
	PrimaryActorTick.bCanEverTick = true;
}

void AProceduralMeshActor::BeginPlay() 
{
	Super::BeginPlay();
}

void AProceduralMeshActor::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);
}
