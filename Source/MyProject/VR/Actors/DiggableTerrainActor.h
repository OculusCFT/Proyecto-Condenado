#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiggableTerrainActor.generated.h"

USTRUCT(BlueprintType)
struct FDigHole
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	float Radius;

	UPROPERTY()
	float Depth;

	FDigHole()
		: Location(FVector::ZeroVector), Radius(0.0f), Depth(0.0f)
	{}

	FDigHole(const FVector& InLocation, float InRadius, float InDepth)
		: Location(InLocation), Radius(InRadius), Depth(InDepth)
	{}
};

UCLASS()
class MYPROJECT_API ADiggableTerrainActor : public AActor
{
	GENERATED_BODY()

public:
	ADiggableTerrainActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TerrainMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HolesMesh;

	// Configuración
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	UMaterialInterface* HoleMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	float MaxHoles = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	bool bUseDecals = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	bool bDeformMesh = false;

	// Estado
	UPROPERTY()
	TArray<FDigHole> DigHoles;

	UPROPERTY()
	TArray<UDecalComponent*> DecalComponents;

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Digging")
	void OnDig(FVector Location, float Radius, float Depth, FVector ImpactNormal);

private:
	void CreateHoleDecal(const FVector& Location, float Radius, const FVector& Normal);
	void DeformMeshAtLocation(const FVector& Location, float Radius, float Depth);
	bool IsLocationAlreadyDug(const FVector& Location, float MinDistance = 10.0f) const;
};