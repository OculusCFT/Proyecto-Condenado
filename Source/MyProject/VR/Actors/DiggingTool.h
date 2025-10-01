#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiggingTool.generated.h"

class UVRGrabComponent;
class UVRDiggingToolComponent;

UCLASS()
class MYPROJECT_API ADiggingTool : public AActor
{
	GENERATED_BODY()

public:
	ADiggingTool();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* HandleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BladeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVRGrabComponent* GrabComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVRDiggingToolComponent* DiggingComponent;

public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnGrabbed();

	UFUNCTION()
	void OnReleased();
};