#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiggingTool.generated.h"

class UVRGrabComponent;
class UVRDiggingToolComponent;
class AParcelaTierra;

UCLASS()
class MYPROJECT_API ADiggingTool : public AActor
{
	GENERATED_BODY()

public:
	ADiggingTool();

protected:
	// ===== COMPONENTS =====
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HandleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BladeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVRGrabComponent* GrabComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVRDiggingToolComponent* DiggingComponent;

	// ===== CONFIG =====
	
	// Radio de detección de parcelas (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	float ParcelaDetectionRadius;

	// Intervalo entre verificaciones de parcela (segundos)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	float ParcelaCheckInterval;

	// ===== STATE =====
	
	// ¿Está siendo agarrada por el jugador?
	UPROPERTY(BlueprintReadOnly, Category = "Digging State")
	bool bIsGrabbed;

	// Tiempo de la última verificación
	float LastParcelaCheckTime;

public:
	// ===== LIFECYCLE =====
	
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	// ===== INTERNAL FUNCTIONS =====
	
	UFUNCTION()
	void OnGrabbed();

	UFUNCTION()
	void OnReleased();

	// Verificar si hay una parcela cercana
	void CheckForNearbyParcela();

	// Llamado cuando se detecta una parcela preparable
	void OnParcelaDetected(AParcelaTierra* Parcela);
};