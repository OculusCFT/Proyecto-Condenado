#include "VRInteractionComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UVRInteractionComponent::UVRInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRInteractionComponent::SetMotionControllers(UMotionControllerComponent* RightGrip, UMotionControllerComponent* LeftGrip)
{
	MotionControllerRightGrip = RightGrip;
	MotionControllerLeftGrip = LeftGrip;
	UE_LOG(LogTemp, Log, TEXT("VRInteractionComponent: Motion controllers set"));
}

bool UVRInteractionComponent::TryGrabWithLeftHand()
{
	if (UActorComponent* NearestComponent = GetGrabComponentNearMotionController(MotionControllerLeftGrip))
	{
		bool bIsHeld = TryGrabComponent(NearestComponent, MotionControllerLeftGrip);
		
		if (bIsHeld)
		{
			HeldComponentLeft = NearestComponent;
			
			// If same object was held by right hand, release it
			if (HeldComponentLeft == HeldComponentRight)
			{
				HeldComponentRight = nullptr;
			}

			OnObjectGrabbed.Broadcast(HeldComponentLeft, false);
			UE_LOG(LogTemp, Log, TEXT("VRInteractionComponent: Left hand grabbed object"));
			return true;
		}
	}
	return false;
}

bool UVRInteractionComponent::TryGrabWithRightHand()
{
	if (UActorComponent* NearestComponent = GetGrabComponentNearMotionController(MotionControllerRightGrip))
	{
		bool bIsHeld = TryGrabComponent(NearestComponent, MotionControllerRightGrip);
		
		if (bIsHeld)
		{
			HeldComponentRight = NearestComponent;
			
			// If same object was held by left hand, release it
			if (HeldComponentRight == HeldComponentLeft)
			{
				HeldComponentLeft = nullptr;
			}

			OnObjectGrabbed.Broadcast(HeldComponentRight, true);
			UE_LOG(LogTemp, Log, TEXT("VRInteractionComponent: Right hand grabbed object"));
			return true;
		}
	}
	return false;
}

bool UVRInteractionComponent::TryReleaseLeftHand()
{
	if (IsValid(HeldComponentLeft))
	{
		bool bReleased = TryReleaseComponent(HeldComponentLeft);
		
		if (bReleased)
		{
			UActorComponent* ReleasedComponent = HeldComponentLeft;
			HeldComponentLeft = nullptr;
			OnObjectReleased.Broadcast(ReleasedComponent, false);
			UE_LOG(LogTemp, Log, TEXT("VRInteractionComponent: Left hand released object"));
			return true;
		}
	}
	return false;
}

bool UVRInteractionComponent::TryReleaseRightHand()
{
	if (IsValid(HeldComponentRight))
	{
		bool bReleased = TryReleaseComponent(HeldComponentRight);
		
		if (bReleased)
		{
			UActorComponent* ReleasedComponent = HeldComponentRight;
			HeldComponentRight = nullptr;
			OnObjectReleased.Broadcast(ReleasedComponent, true);
			UE_LOG(LogTemp, Log, TEXT("VRInteractionComponent: Right hand released object"));
			return true;
		}
	}
	return false;
}

UActorComponent* UVRInteractionComponent::GetGrabComponentNearMotionController(UMotionControllerComponent* MotionController)
{
	if (!MotionController || !IsValid(MotionController) || !GetWorld())
	{
		return nullptr;
	}

	UActorComponent* NearestComponent = nullptr;
	float NearestDistance = GrabRadiusFromGripPosition;
	const FVector ControllerLocation = MotionController->GetComponentLocation();

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		ControllerLocation,
		FQuat::Identity,
		ECC_WorldDynamic,
		FCollisionShape::MakeSphere(GrabRadiusFromGripPosition),
		QueryParams
	);

	if (bHasOverlaps)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (AActor* OverlappedActor = Result.GetActor())
			{
				TArray<UActorComponent*> GrabComponents = OverlappedActor->GetComponentsByTag(UActorComponent::StaticClass(), FName("GrabComponent"));
				
				for (UActorComponent* Component : GrabComponents)
				{
					if (Component && IsValid(Component))
					{
						float Distance = FVector::Dist(ControllerLocation, Component->GetOwner()->GetActorLocation());
						if (Distance < NearestDistance)
						{
							NearestDistance = Distance;
							NearestComponent = Component;
						}
					}
				}
			}
		}
	}

	return NearestComponent;
}

bool UVRInteractionComponent::TryGrabComponent(UActorComponent* GrabComponent, UMotionControllerComponent* MotionController)
{
	if (!GrabComponent || !MotionController)
	{
		return false;
	}

	if (UFunction* TryGrabFunction = GrabComponent->GetClass()->FindFunctionByName(FName("TryGrab")))
	{
		struct FTryGrabParams
		{
			UMotionControllerComponent* Controller;
			bool ReturnValue;
		};

		FTryGrabParams Params;
		Params.Controller = MotionController;
		Params.ReturnValue = false;

		GrabComponent->ProcessEvent(TryGrabFunction, &Params);
		return Params.ReturnValue;
	}

	return false;
}

bool UVRInteractionComponent::TryReleaseComponent(UActorComponent* GrabComponent)
{
	if (!GrabComponent)
	{
		return false;
	}

	if (UFunction* TryReleaseFunction = GrabComponent->GetClass()->FindFunctionByName(FName("TryRelease")))
	{
		struct FTryReleaseParams
		{
			bool ReturnValue;
		};

		FTryReleaseParams Params;
		Params.ReturnValue = false;

		GrabComponent->ProcessEvent(TryReleaseFunction, &Params);
		return Params.ReturnValue;
	}

	return false;
}