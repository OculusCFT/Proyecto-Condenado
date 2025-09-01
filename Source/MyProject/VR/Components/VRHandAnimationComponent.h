#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Engine.h"
#include "VRHandAnimationComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHandAnimationChanged, bool, bIsRightHand, FString, AnimationType, float, Value);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(VR), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UVRHandAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRHandAnimationComponent();

	UPROPERTY(BlueprintAssignable, Category = "VR Hand Events")
	FOnHandAnimationChanged OnHandAnimationChanged;

protected:
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Hand Config")
	TSubclassOf<UAnimInstance> HandAnimBPClass;

	// References
	UPROPERTY()
	USkeletalMeshComponent* HandRight = nullptr;

	UPROPERTY()
	USkeletalMeshComponent* HandLeft = nullptr;

	// Internal state
	bool bHandsInitialized = false;

public:
	virtual void BeginPlay() override;

	// Setup - ESTAS FUNCIONES SON PÚBLICAS AHORA
	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void SetHandMeshes(USkeletalMeshComponent* RightHand, USkeletalMeshComponent* LeftHand);

	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void SetupHandAnimBP();

	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void ForceReinitializeHands();

	// Main Animation Interface
	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void UpdateHandAnimation(bool bRightHand, const FString& AnimationType, float Value);

	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void SetHandPose(bool bRightHand, const FString& PoseName, float Alpha);

	// Individual Animation Controls
	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void SetThumbUpPose(bool bRightHand, float Alpha);

	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void SetPointPose(bool bRightHand, float Alpha);

	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void SetGraspPose(bool bRightHand, float Alpha);

	UFUNCTION(BlueprintCallable, Category = "VR Hands")
	void SetIndexCurl(bool bRightHand, float Alpha);

	// Debug
	UFUNCTION(BlueprintCallable, Category = "VR Debug")
	void DebugDrawHandBones(bool bRightHand);

	// Getters
	UFUNCTION(BlueprintPure, Category = "VR Hands")
	bool AreHandsInitialized() const { return bHandsInitialized; }

private:
	void ConfigureHandMirroring();
	void SetAnimBPVariable(UAnimInstance* AnimInstance, const FString& VariableName, float Value);
	void SetAnimBPVariable(UAnimInstance* AnimInstance, const FString& VariableName, bool Value);
	FString GetAnimBPVariableName(const FString& AnimationType) const;
};