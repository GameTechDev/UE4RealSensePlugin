// This class is intentionally left empty until the Procedural Mesh Component is availabled in C++. 
// Currently, this feature is experimental and only available in Blueprints.
#pragma once

#include "ProceduralMeshActor.generated.h"

UCLASS(BlueprintType)
class AProceduralMeshActor : public AActor
{
	GENERATED_UCLASS_BODY()

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;
};
