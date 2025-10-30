#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyProject/VR/Gameplay/GameplayTypes.h"
#include "Cultivo.generated.h"

// Estados posibles de un cultivo
UENUM(BlueprintType)
enum class ECultivoState : uint8
{
	Semilla UMETA(DisplayName = "Semilla (0-33%)"),
	Creciendo UMETA(DisplayName = "Creciendo (33-99%)"),
	Maduro UMETA(DisplayName = "Maduro (100%)"),
	Seco UMETA(DisplayName = "Seco (Sin riego)")
};

// Delegate para cuando el cultivo cambia de estado
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCultivoStateChanged, ECultivoState, NewState, float, GrowthPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCultivoNeedsWater);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCultivoHarvested);

/**
 * Clase base para todos los cultivos del juego.
 * Maneja:
 * - Crecimiento automático con timer
 * - Sistema de riego (cada 60s)
 * - Estados visuales (Semilla -> Creciendo -> Maduro -> Seco)
 * - Cosecha y valor monetario
 */
UCLASS()
class MYPROJECT_API ACultivo : public AActor
{
	GENERATED_BODY()

public:
	ACultivo();

	// ============================================================
	// EVENTS
	// ============================================================
	
	UPROPERTY(BlueprintAssignable, Category = "Cultivo Events")
	FOnCultivoStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Cultivo Events")
	FOnCultivoNeedsWater OnNeedsWater;

	UPROPERTY(BlueprintAssignable, Category = "Cultivo Events")
	FOnCultivoHarvested OnHarvested;

	// ============================================================
	// COMPONENTS
	// ============================================================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* CultivoMesh;

	// ============================================================
	// CULTIVO CONFIG - AHORA PÚBLICO
	// ============================================================
	
	// Tipo de cultivo (Zanahoria, Tomate, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Config")
	ECultivoType TipoCultivo;

	// Tiempo total de crecimiento en segundos
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Config")
	float TiempoCrecimientoSegundos = 120.0f;

	// Valor cuando se cosecha maduro
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Config")
	int32 ValorCosecha = 20;

	// Cada cuánto necesita riego (segundos)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Config")
	float IntervaloRiego = 60.0f;

	// Tiempo sin riego antes de secarse (segundos)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Config")
	float TiempoAntesDeSecar = 120.0f;

protected:
	// ============================================================
	// VISUAL CONFIG - Mantener protected
	// ============================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Visuals")
	UStaticMesh* MeshSemilla;
    
	// ... resto de meshes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Visuals")
	UStaticMesh* MeshCreciendo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Visuals")
	UStaticMesh* MeshMaduro;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cultivo Visuals")
	UStaticMesh* MeshSeco;

	// ============================================================
	// STATE
	// ============================================================
	
	// Estado actual del cultivo
	UPROPERTY(BlueprintReadOnly, Category = "Cultivo State")
	ECultivoState CurrentState;

	// Tiempo transcurrido desde plantación (segundos)
	UPROPERTY(BlueprintReadOnly, Category = "Cultivo State")
	float TiempoTranscurrido;

	// Última vez que fue regado
	UPROPERTY(BlueprintReadOnly, Category = "Cultivo State")
	float TiempoUltimoRiego;

	// Necesita riego ahora mismo
	UPROPERTY(BlueprintReadOnly, Category = "Cultivo State")
	bool bNecesitaRiego;

	// Ya fue cosechado (destruido después)
	UPROPERTY(BlueprintReadOnly, Category = "Cultivo State")
	bool bFueCosechado;

public:
	// ============================================================
	// LIFECYCLE
	// ============================================================
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================
	// GROWTH SYSTEM
	// ============================================================
	
	// Iniciar crecimiento del cultivo
	UFUNCTION(BlueprintCallable, Category = "Cultivo")
	void StartGrowth();

	// Obtener porcentaje de crecimiento (0-100)
	UFUNCTION(BlueprintPure, Category = "Cultivo")
	float GetGrowthPercent() const;

	// Verificar si está maduro
	UFUNCTION(BlueprintPure, Category = "Cultivo")
	bool IsMature() const { return CurrentState == ECultivoState::Maduro; }

	// Verificar si está seco
	UFUNCTION(BlueprintPure, Category = "Cultivo")
	bool IsDry() const { return CurrentState == ECultivoState::Seco; }

	// ============================================================
	// WATERING SYSTEM
	// ============================================================
	
	// Regar el cultivo
	UFUNCTION(BlueprintCallable, Category = "Cultivo")
	void Water();

	// Verificar si necesita riego
	UFUNCTION(BlueprintPure, Category = "Cultivo")
	bool NeedsWater() const { return bNecesitaRiego; }

	// Obtener tiempo desde último riego
	UFUNCTION(BlueprintPure, Category = "Cultivo")
	float GetTimeSinceLastWater() const;

	// ============================================================
	// HARVEST SYSTEM
	// ============================================================
	
	// Intentar cosechar (solo si está maduro o seco)
	UFUNCTION(BlueprintCallable, Category = "Cultivo")
	bool TryHarvest(int32& OutValue);

	// Obtener valor actual de cosecha (50% si está seco)
	UFUNCTION(BlueprintPure, Category = "Cultivo")
	int32 GetCurrentHarvestValue() const;

	// ============================================================
	// GETTERS
	// ============================================================
	
	UFUNCTION(BlueprintPure, Category = "Cultivo")
	ECultivoState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Cultivo")
	ECultivoType GetTipoCultivo() const { return TipoCultivo; }

	UFUNCTION(BlueprintPure, Category = "Cultivo")
	bool WasHarvested() const { return bFueCosechado; }

private:
	// ============================================================
	// INTERNAL HELPERS
	// ============================================================
	
	// Actualizar estado basado en tiempo transcurrido
	void UpdateGrowthState(float DeltaTime);

	// Actualizar necesidad de riego
	void UpdateWaterNeed(float DeltaTime);

	// Cambiar estado y actualizar visual
	void ChangeState(ECultivoState NewState);

	// Actualizar mesh según estado actual
	void UpdateVisualMesh();

	// Obtener mesh para un estado específico
	UStaticMesh* GetMeshForState(ECultivoState State) const;
};