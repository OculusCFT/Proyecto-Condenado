#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRDiggingToolComponent.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup=(VR), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UVRDiggingToolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRDiggingToolComponent();

protected:
	// Configuración
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	float DigRadius = 15.0f; // Radio pequeño en la punta

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	float DigDepth = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	float MinVelocityToDigCm = 50.0f; // Velocidad mínima en cm/s

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	FName ToolTipSocketName = FName("ToolTip"); // Socket en la punta de la pala

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Digging Config")
	FVector ToolTipOffset = FVector(0.0f, 0.0f, -50.0f); // Si no hay socket

	// Referencias
	UPROPERTY()
	UStaticMeshComponent* ToolMesh = nullptr;

	// Estado
	FVector LastTipPosition;
	FVector CurrentTipVelocity;
	bool bIsDigging = false;

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Digging")
	void SetToolMesh(UStaticMeshComponent* Mesh) { ToolMesh = Mesh; }

	UFUNCTION(BlueprintCallable, Category = "Digging")
	void StartDigging();

	UFUNCTION(BlueprintCallable, Category = "Digging")
	void StopDigging();

	UFUNCTION(BlueprintCallable, Category = "Digging")
	bool TryDigAtLocation(const FVector& Location);

	UFUNCTION(BlueprintPure, Category = "Digging")
	FVector GetToolTipLocation() const;

private:
	void UpdateVelocity(float DeltaTime);
	bool ShouldDig() const;
};