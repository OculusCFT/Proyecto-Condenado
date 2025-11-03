#include "DiggingTool.h"
#include "MyProject/VR/Components/VRGrabComponent.h"
#include "MyProject/VR/Components/VRDiggingToolComponent.h"
#include "ParcelaTierra.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ADiggingTool::ADiggingTool()
{
	PrimaryActorTick.bCanEverTick = true; // ← CAMBIADO: Necesitamos tick para detectar parcelas

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
	
	BladeMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));

	// Grab Component
	GrabComponent = CreateDefaultSubobject<UVRGrabComponent>(TEXT("GrabComponent"));
	GrabComponent->GrabType = EGrabType::Attach;

	// Digging Component
	DiggingComponent = CreateDefaultSubobject<UVRDiggingToolComponent>(TEXT("DiggingComponent"));

	// ===== NUEVAS VARIABLES =====
	bIsGrabbed = false;
	ParcelaDetectionRadius = 150.0f;
	LastParcelaCheckTime = 0.0f;
	ParcelaCheckInterval = 0.2f; // Verificar cada 0.2 segundos
}

void ADiggingTool::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (DiggingComponent && BladeMesh)
	{
		DiggingComponent->SetToolMesh(BladeMesh);
		UE_LOG(LogTemp, Warning, TEXT("DiggingTool: Mesh auto-configured"));
	}

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

// ===== NUEVO: TICK PARA DETECTAR PARCELAS =====
void ADiggingTool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Solo verificar si la pala está agarrada
	if (!bIsGrabbed)
	{
		return;
	}

	// Verificar parcelas cada cierto intervalo (no cada frame)
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastParcelaCheckTime < ParcelaCheckInterval)
	{
		return;
	}

	LastParcelaCheckTime = CurrentTime;

	// Detectar parcela cercana
	CheckForNearbyParcela();
}

// ===== NUEVO: DETECTAR PARCELA CERCANA =====
void ADiggingTool::CheckForNearbyParcela()
{
	if (!BladeMesh)
	{
		return;
	}

	// Punto de inicio: punta de la pala
	FVector Start = BladeMesh->GetComponentLocation();
	FVector End = Start + FVector(0, 0, -100.0f); // Raycast hacia abajo

	// Configurar parámetros de colisión
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;

	FHitResult HitResult;

	// Hacer raycast
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		// Intentar castear a ParcelaTierra
		AParcelaTierra* Parcela = Cast<AParcelaTierra>(HitResult.GetActor());

		if (Parcela)
		{
			OnParcelaDetected(Parcela);
		}
	}
}

// ===== NUEVO: CUANDO SE DETECTA UNA PARCELA =====
void ADiggingTool::OnParcelaDetected(AParcelaTierra* Parcela)
{
	if (!Parcela)
	{
		return;
	}

	// Verificar si la parcela puede ser preparada
	if (!Parcela->CanBePrepared())
	{
		// Ya está preparada o tiene cultivo
		UE_LOG(LogTemp, Verbose, TEXT("DiggingTool: Parcela already prepared or has crop"));
		return;
	}

	// ¡PREPARAR LA TIERRA!
	bool bSuccess = Parcela->PrepareGround();

	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("========== PARCELA PREPARADA! =========="));
		UE_LOG(LogTemp, Warning, TEXT("Location: %s"), *Parcela->GetActorLocation().ToString());

		// Aquí puedes agregar:
		// - Efecto de partículas
		// - Sonido de excavación
		// - Haptic feedback en VR
		// - Animación de la pala

		// Prevenir múltiples preparaciones rápidas
		LastParcelaCheckTime = GetWorld()->GetTimeSeconds() + 1.0f; // Cooldown de 1 segundo
	}
}

void ADiggingTool::OnGrabbed()
{
	UE_LOG(LogTemp, Error, TEXT("========== PALA AGARRADA =========="));
	
	bIsGrabbed = true;
	
	// También podemos seguir usando el sistema de excavación libre
	if (DiggingComponent)
	{
		DiggingComponent->StartDigging();
	}
}

void ADiggingTool::OnReleased()
{
	UE_LOG(LogTemp, Error, TEXT("========== PALA SOLTADA =========="));
	
	bIsGrabbed = false;
	
	if (DiggingComponent)
	{
		DiggingComponent->StopDigging();
	}
}