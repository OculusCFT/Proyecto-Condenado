#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyProject/VR/Gameplay/GameplayTypes.h"
#include "ParcelaTierra.generated.h"

// Forward declaration
class ACultivo;

// Estados de la parcela
UENUM(BlueprintType)
enum class EParcelaState : uint8
{
	SinPreparar UMETA(DisplayName = "Sin Preparar"),
	Preparada UMETA(DisplayName = "Preparada (Surco)"),
	ConCultivo UMETA(DisplayName = "Con Cultivo")
};

// Delegate para eventos de parcela
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParcelaStateChanged, EParcelaState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCultivoPlanted, ACultivo*, PlantedCultivo);

/**
 * Parcela de tierra donde se pueden plantar cultivos.
 * Requiere preparación con pala antes de plantar.
 * Contiene referencia al cultivo actual.
 */
UCLASS()
class MYPROJECT_API AParcelaTierra : public AActor
{
	GENERATED_BODY()

public:
	AParcelaTierra();

	// ============================================================
	// EVENTS
	// ============================================================
	
	UPROPERTY(BlueprintAssignable, Category = "Parcela Events")
	FOnParcelaStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Parcela Events")
	FOnCultivoPlanted OnCultivoPlanted;

protected:
	// ============================================================
	// COMPONENTS
	// ============================================================
	
	// Mesh de la tierra (cambia según estado)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TierraMesh;

	// Punto de spawn para el cultivo
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* CultivoSpawnPoint;

	// ============================================================
	// VISUAL CONFIG
	// ============================================================
	
	// Mesh para tierra sin preparar
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parcela Visuals")
	UStaticMesh* MeshSinPreparar;

	// Mesh para tierra preparada (con surco)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parcela Visuals")
	UStaticMesh* MeshPreparada;

	// ============================================================
	// STATE
	// ============================================================
	
	// Estado actual de la parcela
	UPROPERTY(BlueprintReadOnly, Category = "Parcela State")
	EParcelaState CurrentState;

	// Cultivo actual plantado (null si no hay)
	UPROPERTY(BlueprintReadOnly, Category = "Parcela State")
	ACultivo* CurrentCultivo;

public:
	// ============================================================
	// LIFECYCLE
	// ============================================================
	
	virtual void BeginPlay() override;

	// ============================================================
	// PREPARATION SYSTEM
	// ============================================================
	
	// Preparar la tierra con pala (cavar surco)
	UFUNCTION(BlueprintCallable, Category = "Parcela")
	bool PrepareGround();

	// Verificar si está preparada
	UFUNCTION(BlueprintPure, Category = "Parcela")
	bool IsGroundPrepared() const { return CurrentState == EParcelaState::Preparada; }

	// Verificar si puede ser preparada
	UFUNCTION(BlueprintPure, Category = "Parcela")
	bool CanBePrepared() const { return CurrentState == EParcelaState::SinPreparar; }

	// ============================================================
	// PLANTING SYSTEM
	// ============================================================
	
	// Plantar un cultivo (spawn actor)
	UFUNCTION(BlueprintCallable, Category = "Parcela")
	bool PlantCrop(TSubclassOf<ACultivo> CultivoClass, ECultivoType TipoCultivo);

	// Verificar si puede plantarse
	UFUNCTION(BlueprintPure, Category = "Parcela")
	bool CanPlant() const { return CurrentState == EParcelaState::Preparada; }

	// Verificar si tiene cultivo
	UFUNCTION(BlueprintPure, Category = "Parcela")
	bool HasCultivo() const { return CurrentCultivo != nullptr; }

	// ============================================================
	// HARVEST SYSTEM
	// ============================================================
	
	// Cosechar el cultivo actual
	UFUNCTION(BlueprintCallable, Category = "Parcela")
	bool HarvestCrop(int32& OutValue);

	// Limpiar parcela (después de cosechar)
	UFUNCTION(BlueprintCallable, Category = "Parcela")
	void ClearParcela();

	// ============================================================
	// GETTERS
	// ============================================================
	
	UFUNCTION(BlueprintPure, Category = "Parcela")
	EParcelaState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Parcela")
	ACultivo* GetCurrentCultivo() const { return CurrentCultivo; }

	UFUNCTION(BlueprintPure, Category = "Parcela")
	FVector GetCultivoSpawnLocation() const;

private:
	// ============================================================
	// INTERNAL HELPERS
	// ============================================================
	
	// Cambiar estado de la parcela
	void ChangeState(EParcelaState NewState);

	// Actualizar mesh visual según estado
	void UpdateVisualMesh();

	// Obtener mesh para un estado
	UStaticMesh* GetMeshForState(EParcelaState State) const;
};