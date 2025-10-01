#include "DiggingTool.h"
#include "MyProject/VR/Components/VRGrabComponent.h"
#include "MyProject/VR/Components/VRDiggingToolComponent.h"
#include "Components/StaticMeshComponent.h"

ADiggingTool::ADiggingTool()
{
	PrimaryActorTick.bCanEverTick = false;

	// Handle (mango)
	HandleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandleMesh"));
	RootComponent = HandleMesh;
	HandleMesh->SetSimulatePhysics(true);
	HandleMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	HandleMesh->SetMassOverrideInKg(NAME_None, 2.0f);

	// Blade (hoja/pala)
	BladeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BladeMesh"));
	BladeMesh->SetupAttachment(HandleMesh);
	BladeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Posicionar la hoja en la punta del mango (ajusta según tu mesh)
	BladeMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));

	// Grab Component
	GrabComponent = CreateDefaultSubobject<UVRGrabComponent>(TEXT("GrabComponent"));
	GrabComponent->GrabType = EGrabType::Attach;

	// Digging Component
	DiggingComponent = CreateDefaultSubobject<UVRDiggingToolComponent>(TEXT("DiggingComponent"));
}

void ADiggingTool::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Configurar mesh del componente de excavación
	if (DiggingComponent && BladeMesh)
	{
		DiggingComponent->SetToolMesh(BladeMesh);
		UE_LOG(LogTemp, Warning, TEXT("DiggingTool: Mesh auto-configured"));
	}

	// Conectar eventos de agarre
	if (GrabComponent)
	{
		GrabComponent->OnToolGrabbed.AddDynamic(this, &ADiggingTool::OnGrabbed);
		GrabComponent->OnToolReleased.AddDynamic(this, &ADiggingTool::OnReleased);
		UE_LOG(LogTemp, Warning, TEXT("DiggingTool: Events auto-connected"));
	}
}

void ADiggingTool::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Error, TEXT("========== DiggingTool Ready: %s =========="), *GetName());
}

void ADiggingTool::OnGrabbed()
{
	UE_LOG(LogTemp, Error, TEXT("========== PALA AGARRADA =========="));
	
	if (DiggingComponent)
	{
		DiggingComponent->StartDigging();
	}
}

void ADiggingTool::OnReleased()
{
	UE_LOG(LogTemp, Error, TEXT("========== PALA SOLTADA =========="));
	
	if (DiggingComponent)
	{
		DiggingComponent->StopDigging();
	}
}