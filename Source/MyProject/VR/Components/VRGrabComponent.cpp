#include "VRGrabComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

UVRGrabComponent::UVRGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    // Add the required tag for the interaction system
    ComponentTags.Add(FName("GrabComponent"));
}

void UVRGrabComponent::BeginPlay()
{
    Super::BeginPlay();
    
    GrabbedComponent = GetGrabbableComponent();
    
    if (!GrabbedComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("VRGrabComponent: No valid primitive component found on %s"), 
            *GetOwner()->GetName());
    }
}

void UVRGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsGrabbed && GrabType == EGrabType::PhysicsHandle)
    {
        UpdatePhysicsHandle();
    }
}

bool UVRGrabComponent::TryGrab(UMotionControllerComponent* Controller)
{
    if (bIsGrabbed || !Controller || !GrabbedComponent)
    {
        return false;
    }

    GrabbingController = Controller;
    bIsGrabbed = true;

    // Store original physics state
    bWasSimulatingPhysics = GrabbedComponent->IsSimulatingPhysics();
    OriginalCollisionEnabled = GrabbedComponent->GetCollisionEnabled();

    switch (GrabType)
    {
    case EGrabType::Attach:
        GrabWithAttach(Controller);
        break;
        
    case EGrabType::PhysicsHandle:
        GrabWithPhysicsHandle(Controller);
        break;
        
    case EGrabType::Custom:
        // Override this in Blueprint or child classes
        break;
    }

    // Broadcast grab event
    OnToolGrabbed.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("VRGrabComponent: Object grabbed - %s"), *GetOwner()->GetName());
    return true;
}

bool UVRGrabComponent::TryRelease()
{
    if (!bIsGrabbed)
    {
        return false;
    }

    switch (GrabType)
    {
    case EGrabType::Attach:
        ReleaseFromAttach();
        break;
        
    case EGrabType::PhysicsHandle:
        ReleaseFromPhysicsHandle();
        break;
        
    case EGrabType::Custom:
        // Override this in Blueprint or child classes
        break;
    }

    bIsGrabbed = false;
    GrabbingController = nullptr;
    SetComponentTickEnabled(false);

    // Broadcast release event
    OnToolReleased.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("VRGrabComponent: Object released - %s"), *GetOwner()->GetName());
    return true;
}

UPrimitiveComponent* UVRGrabComponent::GetGrabbableComponent()
{
    // Try to find a primitive component on the owner
    UPrimitiveComponent* PrimComp = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
    
    if (!PrimComp)
    {
        // If not found, try the root component
        PrimComp = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
    }
    
    return PrimComp;
}

void UVRGrabComponent::GrabWithAttach(UMotionControllerComponent* Controller)
{
    if (!GrabbedComponent || !Controller)
    {
        return;
    }

    // Disable physics
    GrabbedComponent->SetSimulatePhysics(false);
    GrabbedComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // Attach to controller
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::KeepWorld,
        EAttachmentRule::KeepWorld,
        EAttachmentRule::KeepWorld,
        false
    );

    if (GrabSocketName != NAME_None)
    {
        GrabbedComponent->AttachToComponent(Controller, AttachRules, GrabSocketName);
    }
    else
    {
        GrabbedComponent->AttachToComponent(Controller, AttachRules);
    }

    // Apply offset if needed
    if (!GrabOffset.IsZero() || !GrabRotationOffset.IsZero())
    {
        FVector CurrentLocation = GrabbedComponent->GetRelativeLocation();
        FRotator CurrentRotation = GrabbedComponent->GetRelativeRotation();
        
        GrabbedComponent->SetRelativeLocationAndRotation(
            CurrentLocation + GrabOffset,
            CurrentRotation + GrabRotationOffset
        );
    }
}

void UVRGrabComponent::GrabWithPhysicsHandle(UMotionControllerComponent* Controller)
{
    if (!GrabbedComponent || !Controller)
    {
        return;
    }

    PhysicsHandle = GetOrCreatePhysicsHandle();
    
    if (!PhysicsHandle)
    {
        UE_LOG(LogTemp, Error, TEXT("VRGrabComponent: Failed to create physics handle"));
        return;
    }

    // Ensure physics is enabled
    if (!GrabbedComponent->IsSimulatingPhysics())
    {
        GrabbedComponent->SetSimulatePhysics(true);
    }

    // Configure physics handle
    PhysicsHandle->LinearDamping = PhysicsHandleLinearDamping;
    PhysicsHandle->LinearStiffness = PhysicsHandleLinearStiffness;
    PhysicsHandle->AngularDamping = PhysicsHandleAngularDamping;
    PhysicsHandle->AngularStiffness = PhysicsHandleAngularStiffness;

    // Grab the component
    FVector GrabLocation = Controller->GetComponentLocation();
    PhysicsHandle->GrabComponentAtLocationWithRotation(
        GrabbedComponent,
        NAME_None,
        GrabLocation,
        Controller->GetComponentRotation()
    );

    SetComponentTickEnabled(true);
}

void UVRGrabComponent::ReleaseFromAttach()
{
    if (!GrabbedComponent)
    {
        return;
    }

    // Detach from controller
    FDetachmentTransformRules DetachRules(
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        EDetachmentRule::KeepWorld,
        false
    );
    
    GrabbedComponent->DetachFromComponent(DetachRules);

    // Restore physics if needed
    if (bSimulatePhysicsOnDrop)
    {
        GrabbedComponent->SetSimulatePhysics(true);
        GrabbedComponent->SetCollisionEnabled(OriginalCollisionEnabled);
        
        // Apply velocity from controller movement
        if (GrabbingController)
        {
            FVector Velocity = GrabbingController->GetComponentVelocity();
            GrabbedComponent->SetPhysicsLinearVelocity(Velocity);
        }
    }
}

void UVRGrabComponent::ReleaseFromPhysicsHandle()
{
    if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
    {
        PhysicsHandle->ReleaseComponent();
    }

    // Restore original physics state if not dropping with physics
    if (!bSimulatePhysicsOnDrop && GrabbedComponent)
    {
        GrabbedComponent->SetSimulatePhysics(bWasSimulatingPhysics);
        GrabbedComponent->SetCollisionEnabled(OriginalCollisionEnabled);
    }
}

void UVRGrabComponent::UpdatePhysicsHandle()
{
    if (!PhysicsHandle || !GrabbingController || !PhysicsHandle->GetGrabbedComponent())
    {
        return;
    }

    FVector TargetLocation = GrabbingController->GetComponentLocation();
    FRotator TargetRotation = GrabbingController->GetComponentRotation();

    PhysicsHandle->SetTargetLocationAndRotation(TargetLocation, TargetRotation);
}

UPhysicsHandleComponent* UVRGrabComponent::GetOrCreatePhysicsHandle()
{
    if (PhysicsHandle)
    {
        return PhysicsHandle;
    }

    // Try to find existing physics handle on owner
    PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();

    // Create one if it doesn't exist
    if (!PhysicsHandle)
    {
        PhysicsHandle = NewObject<UPhysicsHandleComponent>(GetOwner(), UPhysicsHandleComponent::StaticClass());
        PhysicsHandle->RegisterComponent();
    }

    return PhysicsHandle;
}