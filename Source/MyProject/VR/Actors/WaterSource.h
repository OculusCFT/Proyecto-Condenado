#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaterSource.generated.h"

UCLASS()
class MYPROJECT_API AWaterSource : public AActor
{
	GENERATED_BODY()

public:
	AWaterSource();

protected:
	// Mesh visual (plano de agua, barril, etc)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WaterMesh;

	// Partículas de agua (opcional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* WaterParticles;

public:
	virtual void BeginPlay() override;
};