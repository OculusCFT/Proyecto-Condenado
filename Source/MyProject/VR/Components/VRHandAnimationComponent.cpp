#include "VRHandAnimationComponent.h"
#include "DrawDebugHelpers.h"
#include "Animation/Skeleton.h"
#include "Engine/World.h"
#include "TimerManager.h"

UVRHandAnimationComponent::UVRHandAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bHandsInitialized = false;
}

void UVRHandAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// NO llamar SetupHandAnimBP() aquí - se llamará desde VRPawn después
	UE_LOG(LogTemp, Log, TEXT("VRHandAnimationComponent: BeginPlay completed, waiting for manual setup"));
}

void UVRHandAnimationComponent::SetHandMeshes(USkeletalMeshComponent* RightHand, USkeletalMeshComponent* LeftHand)
{
	HandRight = RightHand;
	HandLeft = LeftHand;
	
	UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: Hand meshes set"));
	UE_LOG(LogTemp, Warning, TEXT("HandRight: %s"), HandRight ? TEXT("Valid") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("HandLeft: %s"), HandLeft ? TEXT("Valid") : TEXT("NULL"));
}

void UVRHandAnimationComponent::SetupHandAnimBP()
{
	UE_LOG(LogTemp, Warning, TEXT("=== VRHandAnimationComponent::SetupHandAnimBP STARTED ==="));
	UE_LOG(LogTemp, Warning, TEXT("HandAnimBPClass: %s"), HandAnimBPClass ? *HandAnimBPClass->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("HandRight: %s"), HandRight ? TEXT("Valid") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("HandLeft: %s"), HandLeft ? TEXT("Valid") : TEXT("NULL"));

	if (HandAnimBPClass && HandRight && HandLeft)
	{
		// Check if hands have skeletal mesh assigned
		UE_LOG(LogTemp, Warning, TEXT("HandRight SkeletalMesh: %s"), 
			HandRight->GetSkeletalMeshAsset() ? *HandRight->GetSkeletalMeshAsset()->GetName() : TEXT("NULL"));
		UE_LOG(LogTemp, Warning, TEXT("HandLeft SkeletalMesh: %s"), 
			HandLeft->GetSkeletalMeshAsset() ? *HandLeft->GetSkeletalMeshAsset()->GetName() : TEXT("NULL"));

		// Assign AnimBP to both hands
		HandRight->SetAnimInstanceClass(HandAnimBPClass);
		HandLeft->SetAnimInstanceClass(HandAnimBPClass);
		
		UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: AnimBP assigned to both hands"));
		
		// Wait a frame for AnimBP to initialize properly
		if (UWorld* World = GetWorld())
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				ConfigureHandMirroring();
				bHandsInitialized = true;
				UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: Setup completed with delay"));
			}, 0.1f, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VRHandAnimationComponent: Setup failed - Missing components:"));
		UE_LOG(LogTemp, Error, TEXT("  HandAnimBPClass: %s"), HandAnimBPClass ? TEXT("OK") : TEXT("MISSING"));
		UE_LOG(LogTemp, Error, TEXT("  HandRight: %s"), HandRight ? TEXT("OK") : TEXT("MISSING"));
		UE_LOG(LogTemp, Error, TEXT("  HandLeft: %s"), HandLeft ? TEXT("OK") : TEXT("MISSING"));
		bHandsInitialized = false;
	}
}

void UVRHandAnimationComponent::ConfigureHandMirroring()
{
	UE_LOG(LogTemp, Warning, TEXT("=== ConfigureHandMirroring STARTED ==="));
	
	// Configure left hand for mirroring
	if (UAnimInstance* LeftAnimInstance = HandLeft->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("LEFT HAND AnimInstance found: %s"), *LeftAnimInstance->GetClass()->GetName());
		SetAnimBPVariable(LeftAnimInstance, TEXT("bMirror"), true);
		UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: LEFT HAND mirror set to TRUE"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VRHandAnimationComponent: LEFT HAND AnimInstance is NULL!"));
	}
	
	// Configure right hand (no mirroring)
	if (UAnimInstance* RightAnimInstance = HandRight->GetAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("RIGHT HAND AnimInstance found: %s"), *RightAnimInstance->GetClass()->GetName());
		SetAnimBPVariable(RightAnimInstance, TEXT("bMirror"), false);
		UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: RIGHT HAND mirror set to FALSE"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VRHandAnimationComponent: RIGHT HAND AnimInstance is NULL!"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("=== ConfigureHandMirroring COMPLETED ==="));
}

void UVRHandAnimationComponent::ForceReinitializeHands()
{
	UE_LOG(LogTemp, Warning, TEXT("=== FORCE REINITIALIZE HANDS ==="));
	
	if (HandRight && HandLeft && HandAnimBPClass)
	{
		bHandsInitialized = false;
		
		// Force reassign AnimBP
		HandRight->SetAnimInstanceClass(nullptr);
		HandLeft->SetAnimInstanceClass(nullptr);
		
		// Wait a frame
		if (UWorld* World = GetWorld())
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				// Reassign AnimBP
				HandRight->SetAnimInstanceClass(HandAnimBPClass);
				HandLeft->SetAnimInstanceClass(HandAnimBPClass);
				
				// Wait another frame for AnimBP to initialize
				if (UWorld* World2 = GetWorld())
				{
					FTimerHandle TimerHandle2;
					World2->GetTimerManager().SetTimer(TimerHandle2, [this]()
					{
						ConfigureHandMirroring();
						bHandsInitialized = true;
						UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: FORCE REINIT COMPLETED"));
					}, 0.1f, false);
				}
			}, 0.05f, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("VRHandAnimationComponent: Cannot force reinitialize - missing components"));
	}
}

void UVRHandAnimationComponent::UpdateHandAnimation(bool bRightHand, const FString& AnimationType, float Value)
{
	if (!bHandsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: Hands not initialized yet, skipping animation update"));
		return;
	}

	USkeletalMeshComponent* HandMesh = bRightHand ? HandRight : HandLeft;
	if (!HandMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("VRHandAnimationComponent: HandMesh is NULL for %s hand!"), 
			   bRightHand ? TEXT("RIGHT") : TEXT("LEFT"));
		return;
	}

	UAnimInstance* AnimInstance = HandMesh->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: AnimInstance is null for %s hand"), 
			   bRightHand ? TEXT("Right") : TEXT("Left"));
		return;
	}

	FString VariableName = GetAnimBPVariableName(AnimationType);
	if (VariableName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: Unknown animation type: %s"), *AnimationType);
		return;
	}

	SetAnimBPVariable(AnimInstance, VariableName, Value);
	OnHandAnimationChanged.Broadcast(bRightHand, AnimationType, Value);

	UE_LOG(LogTemp, Verbose, TEXT("VRHandAnimationComponent: %s hand %s animation (%s): %.2f"), 
		   bRightHand ? TEXT("Right") : TEXT("Left"), 
		   *AnimationType, *VariableName, Value);
}

void UVRHandAnimationComponent::SetHandPose(bool bRightHand, const FString& PoseName, float Alpha)
{
	UpdateHandAnimation(bRightHand, PoseName, Alpha);
}

void UVRHandAnimationComponent::SetThumbUpPose(bool bRightHand, float Alpha)
{
	UpdateHandAnimation(bRightHand, TEXT("ThumbUp"), Alpha);
}

void UVRHandAnimationComponent::SetPointPose(bool bRightHand, float Alpha)
{
	UpdateHandAnimation(bRightHand, TEXT("Point"), Alpha);
}

void UVRHandAnimationComponent::SetGraspPose(bool bRightHand, float Alpha)
{
	UpdateHandAnimation(bRightHand, TEXT("Grasp"), Alpha);
}

void UVRHandAnimationComponent::SetIndexCurl(bool bRightHand, float Alpha)
{
	UpdateHandAnimation(bRightHand, TEXT("IndexCurl"), Alpha);
}

FString UVRHandAnimationComponent::GetAnimBPVariableName(const FString& AnimationType) const
{
	if (AnimationType == TEXT("ThumbUp"))
	{
		return TEXT("PoseAlphaThumbUp");
	}
	else if (AnimationType == TEXT("Point"))
	{
		return TEXT("PoseAlphaPoint");
	}
	else if (AnimationType == TEXT("IndexCurl"))
	{
		return TEXT("PoseAlphaIndexCurl");
	}
	else if (AnimationType == TEXT("Grasp"))
	{
		return TEXT("PoseAlphaGrasp");
	}
	
	return FString();
}

void UVRHandAnimationComponent::SetAnimBPVariable(UAnimInstance* AnimInstance, const FString& VariableName, float Value)
{
	if (!AnimInstance)
		return;

	if (FProperty* Property = AnimInstance->GetClass()->FindPropertyByName(*VariableName))
	{
		if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
		{
			FloatProp->SetPropertyValue_InContainer(AnimInstance, Value);
			UE_LOG(LogTemp, Verbose, TEXT("Set float %s to %.2f"), *VariableName, Value);
		}
		else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Property))
		{
			DoubleProp->SetPropertyValue_InContainer(AnimInstance, (double)Value);
			UE_LOG(LogTemp, Verbose, TEXT("Set double %s to %.2f"), *VariableName, Value);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: Property %s found but is not a float/double"), *VariableName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: Property %s not found in AnimBP"), *VariableName);
	}
}

void UVRHandAnimationComponent::SetAnimBPVariable(UAnimInstance* AnimInstance, const FString& VariableName, bool Value)
{
	if (!AnimInstance)
		return;

	if (FProperty* Property = AnimInstance->GetClass()->FindPropertyByName(*VariableName))
	{
		if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
		{
			BoolProp->SetPropertyValue_InContainer(AnimInstance, Value);
			UE_LOG(LogTemp, Warning, TEXT("Set boolean %s to %s"), *VariableName, Value ? TEXT("TRUE") : TEXT("FALSE"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: Property %s found but is not a boolean"), *VariableName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VRHandAnimationComponent: Boolean property %s not found in AnimBP"), *VariableName);
	}
}

void UVRHandAnimationComponent::DebugDrawHandBones(bool bRightHand)
{
	USkeletalMeshComponent* HandMesh = bRightHand ? HandRight : HandLeft;
	if (!HandMesh || !GetWorld())
		return;

	USkeleton* Skeleton = HandMesh->GetSkeletalMeshAsset() ? HandMesh->GetSkeletalMeshAsset()->GetSkeleton() : nullptr;
	if (!Skeleton)
		return;

	const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();

	TArray<FString> ImportantBones = {
		TEXT("hand_r"), TEXT("hand_l"),
		TEXT("thumb_01_r"), TEXT("thumb_01_l"),
		TEXT("thumb_02_r"), TEXT("thumb_02_l"),
		TEXT("thumb_03_r"), TEXT("thumb_03_l"),
		TEXT("index_metacarpal_r"), TEXT("index_metacarpal_l"),
		TEXT("index_01_r"), TEXT("index_01_l"),
		TEXT("index_02_r"), TEXT("index_02_l"),
		TEXT("index_03_r"), TEXT("index_03_l")
	};

	for (const FString& BoneName : ImportantBones)
	{
		FName BoneNameFName(*BoneName);
		int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneNameFName);
		
		if (BoneIndex != INDEX_NONE)
		{
			FVector BoneLocation = HandMesh->GetBoneLocation(BoneNameFName);
			FColor BoneColor = bRightHand ? FColor::Red : FColor::Blue;
			
			DrawDebugSphere(GetWorld(), BoneLocation, 0.5f, 8, BoneColor, false, -1.0f, 0, 0.1f);
			DrawDebugString(GetWorld(), BoneLocation, BoneName, nullptr, BoneColor, 0.0f, true);
		}
	}

	FVector HandLocation = HandMesh->GetComponentLocation();
	FRotator HandRotation = HandMesh->GetComponentRotation();
	
	DrawDebugCoordinateSystem(GetWorld(), HandLocation, HandRotation, 5.0f, false, -1.0f, 0, 1.0f);
	
	FString HandName = bRightHand ? TEXT("RIGHT HAND") : TEXT("LEFT HAND");
	DrawDebugString(GetWorld(), HandLocation + FVector(0, 0, 10), HandName, nullptr, 
		bRightHand ? FColor::Red : FColor::Blue, 0.0f, true);
}