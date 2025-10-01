#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionControllerComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "VRGrabComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnToolGrabbed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnToolReleased);

UENUM(BlueprintType)
enum class EGrabType : uint8
{
    PhysicsHandle UMETA(DisplayName = "Physics Handle"),
    Attach UMETA(DisplayName = "Attach to Hand"),
    Custom UMETA(DisplayName = "Custom")
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(VR), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UVRGrabComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UVRGrabComponent();

    UPROPERTY(BlueprintAssignable, Category = "VR Grab Events")
    FOnToolGrabbed OnToolGrabbed;

    UPROPERTY(BlueprintAssignable, Category = "VR Grab Events")
    FOnToolReleased OnToolReleased;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config")
    EGrabType GrabType = EGrabType::Attach;

protected:
    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config")
    bool bSimulatePhysicsOnDrop = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config")
    FName GrabSocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config")
    FVector GrabOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config")
    FRotator GrabRotationOffset = FRotator::ZeroRotator;

    // Physics Handle Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config|Physics Handle")
    float PhysicsHandleLinearDamping = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config|Physics Handle")
    float PhysicsHandleLinearStiffness = 750.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config|Physics Handle")
    float PhysicsHandleAngularDamping = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Grab Config|Physics Handle")
    float PhysicsHandleAngularStiffness = 1500.0f;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "VR Grab State")
    bool bIsGrabbed = false;

    UPROPERTY(BlueprintReadOnly, Category = "VR Grab State")
    UMotionControllerComponent* GrabbingController = nullptr;

    UPROPERTY()
    UPrimitiveComponent* GrabbedComponent = nullptr;

    UPROPERTY()
    UPhysicsHandleComponent* PhysicsHandle = nullptr;

    // Store original physics state
    bool bWasSimulatingPhysics = false;
    ECollisionEnabled::Type OriginalCollisionEnabled;

public:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main Interface
    UFUNCTION(BlueprintCallable, Category = "VR Grab")
    bool TryGrab(UMotionControllerComponent* Controller);

    UFUNCTION(BlueprintCallable, Category = "VR Grab")
    bool TryRelease();

    // Getters
    UFUNCTION(BlueprintPure, Category = "VR Grab")
    bool IsGrabbed() const { return bIsGrabbed; }

    UFUNCTION(BlueprintPure, Category = "VR Grab")
    UMotionControllerComponent* GetGrabbingController() const { return GrabbingController; }

protected:
    UPrimitiveComponent* GetGrabbableComponent();
    void GrabWithAttach(UMotionControllerComponent* Controller);
    void GrabWithPhysicsHandle(UMotionControllerComponent* Controller);
    void ReleaseFromAttach();
    void ReleaseFromPhysicsHandle();
    void UpdatePhysicsHandle();
    UPhysicsHandleComponent* GetOrCreatePhysicsHandle();
};