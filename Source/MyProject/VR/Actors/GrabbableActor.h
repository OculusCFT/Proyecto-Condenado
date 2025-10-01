#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrabbableActor.generated.h"

class UVRGrabComponent;

UCLASS()
class MYPROJECT_API AGrabbableActor : public AActor
{
	GENERATED_BODY()

public:
	AGrabbableActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVRGrabComponent* GrabComponent;

public:
	virtual void BeginPlay() override;
};