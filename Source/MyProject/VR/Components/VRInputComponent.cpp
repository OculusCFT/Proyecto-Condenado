#include "VRInputComponent.h"
#include "MyProject/VR/Components/VRTeleportComponent.h"
#include "MyProject/VR/Components/VRInteractionComponent.h"
#include "MyProject/VR/Components/VRHandAnimationComponent.h"
#include "MyProject/VR/Pawns/VRPawn.h"
#include "MotionControllerComponent.h"

UVRInputComponent::UVRInputComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRInputComponent::Initialize(UVRTeleportComponent* TeleportComp, UVRInteractionComponent* InteractionComp, 
								   UVRHandAnimationComponent* HandAnimComp, AVRPawn* Pawn)
{
	TeleportComponent = TeleportComp;
	InteractionComponent = InteractionComp;
	HandAnimationComponent = HandAnimComp;
	OwnerPawn = Pawn;
	
	UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Initialized with all component references"));
}

void UVRInputComponent::SetupInputBindings(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Teleport and movement
		if (IA_Move)
		{
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Started, this, &UVRInputComponent::OnMoveStarted);
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &UVRInputComponent::OnMoveTriggered);
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &UVRInputComponent::OnMoveCompleted);
		}

		if (IA_Turn)
		{
			EnhancedInputComponent->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &UVRInputComponent::OnTurnTriggered);
		}

		// Grab actions
		if (IA_Grab_Left_Pressed)
		{
			EnhancedInputComponent->BindAction(IA_Grab_Left_Pressed, ETriggerEvent::Triggered, this, &UVRInputComponent::OnGrabLeftPressed);
		}

		if (IA_Grab_Left_Released)
		{
			EnhancedInputComponent->BindAction(IA_Grab_Left_Released, ETriggerEvent::Triggered, this, &UVRInputComponent::OnGrabLeftReleased);
		}

		if (IA_Grab_Right_Pressed)
		{
			EnhancedInputComponent->BindAction(IA_Grab_Right_Pressed, ETriggerEvent::Triggered, this, &UVRInputComponent::OnGrabRightPressed);
		}

		if (IA_Grab_Right_Released)
		{
			EnhancedInputComponent->BindAction(IA_Grab_Right_Released, ETriggerEvent::Triggered, this, &UVRInputComponent::OnGrabRightReleased);
		}

		// Hand Grasp (existing)
		if (IA_Hand_Grasp_Right)
		{
			EnhancedInputComponent->BindAction(IA_Hand_Grasp_Right, ETriggerEvent::Triggered, this, &UVRInputComponent::OnHandGraspRight);
		}

		if (IA_Hand_Grasp_Left)
		{
			EnhancedInputComponent->BindAction(IA_Hand_Grasp_Left, ETriggerEvent::Triggered, this, &UVRInputComponent::OnHandGraspLeft);
		}

		// Menu actions
		if (IA_Menu_Toggle_Left)
		{
			EnhancedInputComponent->BindAction(IA_Menu_Toggle_Left, ETriggerEvent::Triggered, this, &UVRInputComponent::OnMenuToggleLeft);
		}

		if (IA_Menu_Toggle_Right)
		{
			EnhancedInputComponent->BindAction(IA_Menu_Toggle_Right, ETriggerEvent::Triggered, this, &UVRInputComponent::OnMenuToggleRight);
		}

		// Hand animation actions
		if (IA_Hand_ThumbUp_Right)
		{
			EnhancedInputComponent->BindAction(IA_Hand_ThumbUp_Right, ETriggerEvent::Started, this, &UVRInputComponent::OnHandThumbUpRightStarted);
			EnhancedInputComponent->BindAction(IA_Hand_ThumbUp_Right, ETriggerEvent::Completed, this, &UVRInputComponent::OnHandThumbUpRightCompleted);
		}

		if (IA_Hand_ThumbUp_Left)
		{
			EnhancedInputComponent->BindAction(IA_Hand_ThumbUp_Left, ETriggerEvent::Started, this, &UVRInputComponent::OnHandThumbUpLeftStarted);
			EnhancedInputComponent->BindAction(IA_Hand_ThumbUp_Left, ETriggerEvent::Completed, this, &UVRInputComponent::OnHandThumbUpLeftCompleted);
		}

		if (IA_Hand_Point_Right)
		{
			EnhancedInputComponent->BindAction(IA_Hand_Point_Right, ETriggerEvent::Started, this, &UVRInputComponent::OnHandPointRightStarted);
			EnhancedInputComponent->BindAction(IA_Hand_Point_Right, ETriggerEvent::Completed, this, &UVRInputComponent::OnHandPointRightCompleted);
		}

		if (IA_Hand_Point_Left)
		{
			EnhancedInputComponent->BindAction(IA_Hand_Point_Left, ETriggerEvent::Started, this, &UVRInputComponent::OnHandPointLeftStarted);
			EnhancedInputComponent->BindAction(IA_Hand_Point_Left, ETriggerEvent::Completed, this, &UVRInputComponent::OnHandPointLeftCompleted);
		}

		if (IA_Hand_IndexCurl_Right)
		{
			EnhancedInputComponent->BindAction(IA_Hand_IndexCurl_Right, ETriggerEvent::Triggered, this, &UVRInputComponent::OnHandIndexCurlRight);
		}

		if (IA_Hand_IndexCurl_Left)
		{
			EnhancedInputComponent->BindAction(IA_Hand_IndexCurl_Left, ETriggerEvent::Triggered, this, &UVRInputComponent::OnHandIndexCurlLeft);
		}

		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Input bindings setup complete"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VRInputComponent: Enhanced Input Component not found"));
	}
}

// INPUT EVENT HANDLERS

void UVRInputComponent::OnMoveStarted(const FInputActionValue& Value)
{
	if (TeleportComponent)
	{
		TeleportComponent->StartTeleportTrace();
	}
}

void UVRInputComponent::OnMoveTriggered(const FInputActionValue& Value)
{
	if (TeleportComponent && OwnerPawn)
	{
		if (UMotionControllerComponent* RightAimController = OwnerPawn->GetRightAimController())
		{
			const FVector StartPos = RightAimController->GetComponentLocation();
			const FVector ForwardVector = RightAimController->GetForwardVector();
			TeleportComponent->UpdateTeleportTrace(StartPos, ForwardVector);
		}
	}
}

void UVRInputComponent::OnMoveCompleted(const FInputActionValue& Value)
{
	if (TeleportComponent)
	{
		TeleportComponent->EndTeleportTrace();
		TeleportComponent->TryExecuteTeleport();
	}
}

void UVRInputComponent::OnTurnTriggered(const FInputActionValue& Value)
{
	const float TurnValue = Value.Get<float>();
	const bool bRightTurn = TurnValue > 0.0f;
	PerformSnapTurn(bRightTurn);
}

void UVRInputComponent::OnGrabLeftPressed(const FInputActionValue& Value)
{
	if (InteractionComponent)
	{
		InteractionComponent->TryGrabWithLeftHand();
	}
}

void UVRInputComponent::OnGrabLeftReleased(const FInputActionValue& Value)
{
	if (InteractionComponent)
	{
		InteractionComponent->TryReleaseLeftHand();
	}
}

void UVRInputComponent::OnGrabRightPressed(const FInputActionValue& Value)
{
	if (InteractionComponent)
	{
		InteractionComponent->TryGrabWithRightHand();
	}
}

void UVRInputComponent::OnGrabRightReleased(const FInputActionValue& Value)
{
	if (InteractionComponent)
	{
		InteractionComponent->TryReleaseRightHand();
	}
}

void UVRInputComponent::OnHandGraspRight(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		const float GraspValue = Value.Get<float>();
		HandAnimationComponent->SetGraspPose(true, GraspValue);
	}
}

void UVRInputComponent::OnHandGraspLeft(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		const float GraspValue = Value.Get<float>();
		HandAnimationComponent->SetGraspPose(false, GraspValue);
	}
}

void UVRInputComponent::PerformSnapTurn(bool bRightTurn)
{
	if (!OwnerPawn)
	{
		return;
	}

	const float SnapTurnDegrees = 45.0f; // Could be made configurable
	const float TurnDirection = bRightTurn ? 1.0f : -1.0f;
	const float TurnAmount = SnapTurnDegrees * TurnDirection;

	// Get VROrigin from the pawn
	if (USceneComponent* VROrigin = OwnerPawn->GetRootComponent())
	{
		FRotator CurrentRotation = VROrigin->GetComponentRotation();
		CurrentRotation.Yaw += TurnAmount;
		VROrigin->SetWorldRotation(CurrentRotation);

		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Snap turn performed - Direction: %s, Amount: %.1f degrees"), 
			bRightTurn ? TEXT("Right") : TEXT("Left"), SnapTurnDegrees);
	}
}

// MENU INPUT HANDLERS
void UVRInputComponent::OnMenuToggleLeft(const FInputActionValue& Value)
{
	// TODO: Implement menu system when UI components are created
	UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Left menu toggle pressed"));
}

void UVRInputComponent::OnMenuToggleRight(const FInputActionValue& Value)
{
	// TODO: Implement menu system when UI components are created
	UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Right menu toggle pressed"));
}

// HAND ANIMATION INPUT HANDLERS
void UVRInputComponent::OnHandThumbUpRightStarted(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetThumbUpPose(true, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Right hand thumb up started"));
	}
}

void UVRInputComponent::OnHandThumbUpRightCompleted(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetThumbUpPose(true, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Right hand thumb up completed"));
	}
}

void UVRInputComponent::OnHandThumbUpLeftStarted(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetThumbUpPose(false, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Left hand thumb up started"));
	}
}

void UVRInputComponent::OnHandThumbUpLeftCompleted(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetThumbUpPose(false, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Left hand thumb up completed"));
	}
}

void UVRInputComponent::OnHandPointRightStarted(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetPointPose(true, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Right hand point started"));
	}
}

void UVRInputComponent::OnHandPointRightCompleted(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetPointPose(true, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Right hand point completed"));
	}
}

void UVRInputComponent::OnHandPointLeftStarted(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetPointPose(false, 1.0f);
		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Left hand point started"));
	}
}

void UVRInputComponent::OnHandPointLeftCompleted(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		HandAnimationComponent->SetPointPose(false, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("VRInputComponent: Left hand point completed"));
	}
}

void UVRInputComponent::OnHandIndexCurlRight(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		const float CurlValue = Value.Get<float>();
		HandAnimationComponent->SetIndexCurl(true, CurlValue);
	}
}

void UVRInputComponent::OnHandIndexCurlLeft(const FInputActionValue& Value)
{
	if (HandAnimationComponent)
	{
		const float CurlValue = Value.Get<float>();
		HandAnimationComponent->SetIndexCurl(false, CurlValue);
	}
}