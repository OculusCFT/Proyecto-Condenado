#include "GrabbableActor.h"
#include "MyProject/VR/Components/VRGrabComponent.h"
#include "Components/StaticMeshComponent.h"

AGrabbableActor::AGrabbableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
    
	// Setup physics
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
	MeshComponent->SetMassOverrideInKg(NAME_None, 1.0f);

	// Create grab component
	GrabComponent = CreateDefaultSubobject<UVRGrabComponent>(TEXT("GrabComponent"));
}

void AGrabbableActor::BeginPlay()
{
	Super::BeginPlay();
}