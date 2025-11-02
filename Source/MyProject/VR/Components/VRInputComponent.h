#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "VRInputComponent.generated.h"

// Forward declarations
class UVRTeleportComponent;
class UVRInteractionComponent;
class UVRHandAnimationComponent;
class AVRPawn;
class UMotionControllerComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup=(VR), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UVRInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRInputComponent();

protected:
	// ============================================================
	// INPUT ACTIONS
	// ============================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Turn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Grab_Left_Pressed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Grab_Left_Released;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Grab_Right_Pressed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Grab_Right_Released;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Hand_Grasp_Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Hand_Grasp_Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Menu_Toggle_Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Menu_Toggle_Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Hand_ThumbUp_Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Hand_ThumbUp_Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Hand_Point_Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Hand_Point_Left;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Hand_IndexCurl_Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Input Actions")
	UInputAction* IA_Hand_IndexCurl_Left;

	// ============================================================
	// COMPONENT REFERENCES
	// ============================================================
	
	UPROPERTY()
	UVRTeleportComponent* TeleportComponent = nullptr;

	UPROPERTY()
	UVRInteractionComponent* InteractionComponent = nullptr;

	UPROPERTY()
	UVRHandAnimationComponent* HandAnimationComponent = nullptr;

	UPROPERTY()
	AVRPawn* OwnerPawn = nullptr;

public:
	// ============================================================
	// SETUP
	// ============================================================
	
	UFUNCTION(BlueprintCallable, Category = "VR Input")
	void Initialize(UVRTeleportComponent* TeleportComp, UVRInteractionComponent* InteractionComp, 
					UVRHandAnimationComponent* HandAnimComp, AVRPawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "VR Input")
	void SetupInputBindings(UInputComponent* PlayerInputComponent);

	// ============================================================
	// INPUT EVENT HANDLERS
	// ============================================================
	
	UFUNCTION()
	void OnMoveStarted(const FInputActionValue& Value);

	UFUNCTION()
	void OnMoveTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void OnMoveCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void OnGrabLeftPressed(const FInputActionValue& Value);

	UFUNCTION()
	void OnGrabLeftReleased(const FInputActionValue& Value);

	UFUNCTION()
	void OnGrabRightPressed(const FInputActionValue& Value);

	UFUNCTION()
	void OnGrabRightReleased(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandGraspRight(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandGraspLeft(const FInputActionValue& Value);

	UFUNCTION()
	void OnMenuToggleLeft(const FInputActionValue& Value);

	UFUNCTION()
	void OnMenuToggleRight(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandThumbUpRightStarted(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandThumbUpRightCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandThumbUpLeftStarted(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandThumbUpLeftCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandPointRightStarted(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandPointRightCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandPointLeftStarted(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandPointLeftCompleted(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandIndexCurlRight(const FInputActionValue& Value);

	UFUNCTION()
	void OnHandIndexCurlLeft(const FInputActionValue& Value);
};