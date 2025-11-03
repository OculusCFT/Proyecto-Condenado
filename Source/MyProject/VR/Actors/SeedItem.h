// ==================================================================
// SeedItem.h
// Semilla física que se puede agarrar y plantar
// ==================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyProject/VR/Gameplay/GameplayTypes.h"
#include "SeedItem.generated.h"

class UVRGrabComponent;
class AParcelaTierra;
class ACultivo;

UCLASS()
class MYPROJECT_API ASeedItem : public AActor
{
	GENERATED_BODY()

public:
	ASeedItem();

protected:
	// ============================================================
	// COMPONENTS
	// ============================================================
	
	// Mesh de la semilla (pequeño objeto físico)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SeedMesh;

	// Componente para agarrar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVRGrabComponent* GrabComponent;

	// ============================================================
	// CONFIG
	// ============================================================
	
	// Tipo de cultivo que plantará
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seed Config")
	ECultivoType CultivoType;

	// Clase del cultivo a spawnear
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seed Config")
	TSubclassOf<ACultivo> CultivoClass;

	// Radio de detección de parcela (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seed Config")
	float PlantingRadius;

	// Altura máxima para plantar (cm desde el suelo)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seed Config")
	float MaxPlantingHeight;

	// ============================================================
	// STATE
	// ============================================================
	
	// ¿Está siendo agarrada?
	UPROPERTY(BlueprintReadOnly, Category = "Seed State")
	bool bIsGrabbed;

	// ¿Ya fue plantada? (para evitar múltiples plantas)
	UPROPERTY(BlueprintReadOnly, Category = "Seed State")
	bool bWasPlanted;

	// Tiempo de última verificación
	float LastCheckTime;

public:
	// ============================================================
	// LIFECYCLE
	// ============================================================
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// ============================================================
	// GRAB EVENTS
	// ============================================================
	
	UFUNCTION()
	void OnGrabbed();

	UFUNCTION()
	void OnReleased();

	// ============================================================
	// PLANTING LOGIC
	// ============================================================
	
	// Verificar si hay parcela preparada cerca
	void CheckForPlantableGround();

	// Intentar plantar en una parcela
	bool TryPlantOnParcela(AParcelaTierra* Parcela);

	// Verificar si está a buena altura para plantar
	bool IsAtPlantingHeight() const;
};