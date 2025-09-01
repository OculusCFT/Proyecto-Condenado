#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionControllerComponent.h"
#include "VRInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectGrabbed, UActorComponent*, GrabbedComponent, bool, bIsRightHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectReleased, UActorComponent*, ReleasedComponent, bool, bIsRightHand);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(VR), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UVRInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRInteractionComponent();

	UPROPERTY(BlueprintAssignable, Category = "VR Interaction Events")
	FOnObjectGrabbed OnObjectGrabbed;

	UPROPERTY(BlueprintAssignable, Category = "VR Interaction Events")
	FOnObjectReleased OnObjectReleased;

protected:
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction Config")
	float GrabRadiusFromGripPosition = 15.0f;

	// State
	UPROPERTY(BlueprintReadOnly, Category = "VR Interaction State")
	UActorComponent* HeldComponentLeft = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "VR Interaction State")
	UActorComponent* HeldComponentRight = nullptr;

	// References
	UPROPERTY()
	UMotionControllerComponent* MotionControllerRightGrip = nullptr;

	UPROPERTY()
	UMotionControllerComponent* MotionControllerLeftGrip = nullptr;

public:
	// Setup
	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	void SetMotionControllers(UMotionControllerComponent* RightGrip, UMotionControllerComponent* LeftGrip);

	// Main Interface
	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	bool TryGrabWithLeftHand();

	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	bool TryGrabWithRightHand();

	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	bool TryReleaseLeftHand();

	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	bool TryReleaseRightHand();

	// Getters
	UFUNCTION(BlueprintPure, Category = "VR Interaction")
	UActorComponent* GetHeldComponentLeft() const { return HeldComponentLeft; }

	UFUNCTION(BlueprintPure, Category = "VR Interaction")
	UActorComponent* GetHeldComponentRight() const { return HeldComponentRight; }

	UFUNCTION(BlueprintPure, Category = "VR Interaction")
	bool IsLeftHandHolding() const { return IsValid(HeldComponentLeft); }

	UFUNCTION(BlueprintPure, Category = "VR Interaction")
	bool IsRightHandHolding() const { return IsValid(HeldComponentRight); }

private:
	UActorComponent* GetGrabComponentNearMotionController(UMotionControllerComponent* MotionController);
	bool TryGrabComponent(UActorComponent* GrabComponent, UMotionControllerComponent* MotionController);
	bool TryReleaseComponent(UActorComponent* GrabComponent);
};