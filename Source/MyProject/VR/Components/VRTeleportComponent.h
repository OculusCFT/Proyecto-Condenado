#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "VRTeleportComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportComplete, FVector, TeleportLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportValidationChanged, bool, bIsValid);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(VR), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UVRTeleportComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRTeleportComponent();

	UPROPERTY(BlueprintAssignable, Category = "VR Teleport Events")
	FOnTeleportComplete OnTeleportComplete;

	UPROPERTY(BlueprintAssignable, Category = "VR Teleport Events")
	FOnTeleportValidationChanged OnTeleportValidationChanged;

protected:
	static constexpr float MIN_TELEPORT_DISTANCE = 50.0f;
	static constexpr float MAX_TELEPORT_SURFACE_ANGLE = 45.0f;
	static constexpr float SURFACE_TRACE_DISTANCE = 100.0f;

	// Teleport Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Teleport Config")
	FVector TeleportProjectPointToNavigationQueryExtent = FVector(500.0f, 500.0f, 500.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Teleport Config")
	float TeleportLaunchVelocity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Teleport Config")
	float TeleportProjectileRadius = 3.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Teleport Config")
	TSubclassOf<AActor> TeleportVisualizerClass;

	// Teleport State
	UPROPERTY(BlueprintReadOnly, Category = "VR Teleport State")
	FVector ProjectedTeleportLocation;

	UPROPERTY(BlueprintReadOnly, Category = "VR Teleport State")
	bool bValidTeleportLocation = false;

	UPROPERTY(BlueprintReadOnly, Category = "VR Teleport State")
	bool bTeleportTraceActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "VR Teleport State")
	TArray<FVector> TeleportTracePathPositions;

	// References
	UPROPERTY()
	AActor* TeleportVisualizerReference = nullptr;

	UPROPERTY()
	UNiagaraComponent* TeleportTraceNiagaraSystem = nullptr;

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Main Teleport Interface
	UFUNCTION(BlueprintCallable, Category = "VR Teleport")
	void StartTeleportTrace();

	UFUNCTION(BlueprintCallable, Category = "VR Teleport")
	void UpdateTeleportTrace(const FVector& StartPos, const FVector& ForwardVector);

	UFUNCTION(BlueprintCallable, Category = "VR Teleport")
	void EndTeleportTrace();

	UFUNCTION(BlueprintCallable, Category = "VR Teleport")
	bool TryExecuteTeleport();

	// Getters
	UFUNCTION(BlueprintPure, Category = "VR Teleport")
	FVector GetCurrentTeleportLocation() const { return bValidTeleportLocation ? ProjectedTeleportLocation : FVector::ZeroVector; }

	UFUNCTION(BlueprintPure, Category = "VR Teleport")
	bool IsTeleportTraceActive() const { return bTeleportTraceActive; }

	UFUNCTION(BlueprintPure, Category = "VR Teleport")
	bool IsValidTeleportLocation() const { return bValidTeleportLocation; }

	// Setup
	UFUNCTION(BlueprintCallable, Category = "VR Teleport")
	void SetTeleportNiagaraSystem(UNiagaraComponent* NiagaraComponent);

private:
	bool IsValidTeleportLocationInternal(const FVector& Location) const;
	bool IsValidSurfaceAngle(const FVector& Location) const;
	void CreateTeleportVisualizer();
	void DestroyTeleportVisualizer();
	void UpdateTeleportValidation(bool bNewValidState);
	void UpdateTeleportVisualization(const TArray<FVector>& PathPositions, bool bIsValidLocation, const FVector& TargetLocation);
};