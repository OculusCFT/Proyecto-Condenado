// ==================================================================
// WateringCan.h
// Regadera que se puede llenar y usar para regar cultivos
// ==================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WateringCan.generated.h"

class UVRGrabComponent;
class ACultivo;

UCLASS()
class MYPROJECT_API AWateringCan : public AActor
{
	GENERATED_BODY()

public:
	AWateringCan();

protected:
	// ============================================================
	// COMPONENTS
	// ============================================================
	
	// Mesh del cuerpo de la regadera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CanMesh;

	// Mesh del pico/boquilla (donde sale el agua)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* SpoutMesh;

	// Punto de spawn de partículas de agua
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* WaterSpawnPoint;

	// Componente para agarrar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVRGrabComponent* GrabComponent;

	// ============================================================
	// CONFIG - WATER SYSTEM
	// ============================================================
	
	// Capacidad máxima de agua (litros/unidades)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watering Config")
	float MaxWaterCapacity;

	// Agua consumida por cada uso
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watering Config")
	float WaterPerUse;

	// Radio de detección de cultivos (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watering Config")
	float WateringRadius;

	// ¿Necesita estar inclinada para regar? (más realista)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watering Config")
	bool bRequireTilt;

	// Ángulo mínimo de inclinación para regar (grados)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Watering Config")
	float MinTiltAngle;

	// ============================================================
	// CONFIG - REFILL SYSTEM
	// ============================================================
	
	// Radio para detectar fuente de agua (río/pozo)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refill Config")
	float RefillRadius;

	// Tag que identifica fuentes de agua (ej: "WaterSource")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refill Config")
	FName WaterSourceTag;

	// Velocidad de llenado (unidades por segundo)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refill Config")
	float RefillRate;

	// ============================================================
	// VISUAL/AUDIO CONFIG
	// ============================================================
	
	// Partículas de agua saliendo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* WaterParticles;

	// Sonido al regar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* WateringSound;

	// Sonido al llenar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* RefillSound;

	// ============================================================
	// STATE
	// ============================================================
	
	// Cantidad actual de agua
	UPROPERTY(BlueprintReadOnly, Category = "Watering State")
	float CurrentWater;

	// ¿Está siendo agarrada?
	UPROPERTY(BlueprintReadOnly, Category = "Watering State")
	bool bIsGrabbed;

	// ¿Está regando actualmente?
	UPROPERTY(BlueprintReadOnly, Category = "Watering State")
	bool bIsWatering;

	// ¿Está llenándose?
	UPROPERTY(BlueprintReadOnly, Category = "Watering State")
	bool bIsRefilling;

	// Tiempo de última verificación
	float LastCheckTime;

	// Último cultivo regado (cooldown)
	UPROPERTY()
	ACultivo* LastWateredCrop;

	float LastWaterTime;

public:
	// ============================================================
	// LIFECYCLE
	// ============================================================
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================
	// WATER MANAGEMENT
	// ============================================================
	
	// Obtener porcentaje de agua (0-1)
	UFUNCTION(BlueprintPure, Category = "Watering")
	float GetWaterPercentage() const;

	// ¿Tiene agua suficiente para regar?
	UFUNCTION(BlueprintPure, Category = "Watering")
	bool HasWater() const;

	// ¿Está llena?
	UFUNCTION(BlueprintPure, Category = "Watering")
	bool IsFull() const;

	// ¿Está vacía?
	UFUNCTION(BlueprintPure, Category = "Watering")
	bool IsEmpty() const;

private:
	// ============================================================
	// INTERNAL FUNCTIONS
	// ============================================================
	
	UFUNCTION()
	void OnGrabbed();

	UFUNCTION()
	void OnReleased();

	// Verificar si está inclinada correctamente
	bool IsProperlyTilted() const;

	// Buscar cultivos cercanos para regar
	void CheckForCropsToWater();

	// Regar un cultivo específico
	bool WaterCrop(ACultivo* Crop);

	// Buscar fuente de agua cercana
	void CheckForWaterSource();

	// Llenar la regadera
	void RefillWater(float DeltaTime);

	// Consumir agua
	bool ConsumeWater(float Amount);

	// Efectos visuales/audio
	void StartWateringEffects();
	void StopWateringEffects();
	void PlayRefillEffects();
};