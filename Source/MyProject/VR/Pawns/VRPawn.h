#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionControllerComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "NiagaraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "VRPawn.generated.h"

// Forward declarations
class UVRTeleportComponent;
class UVRInteractionComponent;
class UVRHandAnimationComponent;
class UVRInputComponent;

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API AVRPawn : public APawn
{
	GENERATED_BODY()

public:
	AVRPawn();
	
	// Tipo de movimiento: false = Teleport, true = Smooth
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Locomotion")
	bool bUseSmoothLocomotion = true;


protected:
	// VR Core Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	USceneComponent* VROrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	UStaticMeshComponent* HeadMountedDisplayMesh;

	// VR System Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR System Components")
	UVRTeleportComponent* TeleportComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR System Components")
	UVRInteractionComponent* InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR System Components")
	UVRHandAnimationComponent* HandAnimationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR System Components")
	UVRInputComponent* VRInputComponent;

	// Controllers
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Controllers")
	UMotionControllerComponent* MotionControllerRightAim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Controllers")
	UMotionControllerComponent* MotionControllerLeftAim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Controllers")
	UMotionControllerComponent* MotionControllerRightGrip;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Controllers")
	UMotionControllerComponent* MotionControllerLeftGrip;

	// Hands
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Hands")
	USkeletalMeshComponent* HandRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Hands")
	USkeletalMeshComponent* HandLeft;

	// Other VR Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Visualization")
	UStaticMeshComponent* XRDeviceVisualizationRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Visualization")
	UStaticMeshComponent* XRDeviceVisualizationLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Interaction")
	UWidgetInteractionComponent* WidgetInteractionRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Interaction")
	UWidgetInteractionComponent* WidgetInteractionLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Effects")
	UNiagaraComponent* TeleportTraceNiagaraSystem;

	// Input Configuration
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input")
	UInputMappingContext* HandsMappingContext;

	// Debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Debug")
	bool bDebugHandBones = true;

	// ============================================================
	// LOCOMOTION CONFIG
	// ============================================================
	

	// Velocidad de movimiento (cm/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Locomotion")
	float MovementSpeed = 100.0f;

	// Threshold mínimo del joystick para iniciar movimiento
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Locomotion")
	float MovementThreshold = 0.2f;

public:
	// ============================================================
	// LOCOMOTION STATE (public para que VRInputComponent pueda modificarlo)
	// ============================================================
	
	// Estado actual del movimiento
	UPROPERTY(BlueprintReadOnly, Category = "VR Locomotion")
	FVector2D CurrentMovementInput;

	// ============================================================
	// LIFECYCLE
	// ============================================================
	
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================
	// SMOOTH LOCOMOTION
	// ============================================================
	
	// Aplicar movimiento suave basado en input del joystick
	UFUNCTION(BlueprintCallable, Category = "VR Locomotion")
	void ApplySmoothMovement(FVector2D MovementInput, float DeltaTime);

	// Obtener dirección de movimiento basada en la cámara
	UFUNCTION(BlueprintPure, Category = "VR Locomotion")
	FVector GetCameraRelativeMovementDirection(FVector2D Input) const;

	// Component Getters
	UFUNCTION(BlueprintPure, Category = "VR Components")
	UVRTeleportComponent* GetTeleportComponent() const { return TeleportComponent; }

	UFUNCTION(BlueprintPure, Category = "VR Components")
	UVRInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	UFUNCTION(BlueprintPure, Category = "VR Components")
	UVRHandAnimationComponent* GetHandAnimationComponent() const { return HandAnimationComponent; }

	UFUNCTION(BlueprintPure, Category = "VR Components")
	UVRInputComponent* GetVRInputComponent() const { return VRInputComponent; }

	// VR Hardware Getters
	UFUNCTION(BlueprintPure, Category = "VR Hardware")
	UMotionControllerComponent* GetRightGripController() const { return MotionControllerRightGrip; }

	UFUNCTION(BlueprintPure, Category = "VR Hardware")
	UMotionControllerComponent* GetLeftGripController() const { return MotionControllerLeftGrip; }

	UFUNCTION(BlueprintPure, Category = "VR Hardware")
	UMotionControllerComponent* GetRightAimController() const { return MotionControllerRightAim; }

	UFUNCTION(BlueprintPure, Category = "VR Hardware")
	UMotionControllerComponent* GetLeftAimController() const { return MotionControllerLeftAim; }

private:
	void SetupComponentHierarchy();
	void InitializeVRComponents();
	void SetupInputMappingContexts();
	class UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};