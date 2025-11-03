// ==================================================================
// SeedItem.cpp
// Implementación de semilla física plantable
// ==================================================================

#include "SeedItem.h"
#include "ParcelaTierra.h"
#include "Cultivo.h"
#include "MyProject/VR/Components/VRGrabComponent.h"
#include "MyProject/VR/GameModes/HarvestHavenGameManager.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ASeedItem::ASeedItem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Crear mesh de semilla (pequeño objeto)
	SeedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeedMesh"));
	RootComponent = SeedMesh;
	
	// Configurar física
	SeedMesh->SetSimulatePhysics(true);
	SeedMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	SeedMesh->SetMassOverrideInKg(NAME_None, 0.1f); // Muy ligera
	SeedMesh->SetEnableGravity(true);

	// Grab component
	GrabComponent = CreateDefaultSubobject<UVRGrabComponent>(TEXT("GrabComponent"));
	GrabComponent->GrabType = EGrabType::Attach;

	// Config por defecto
	PlantingRadius = 80.0f;
	MaxPlantingHeight = 100.0f; // 1 metro sobre el suelo
	CultivoType = ECultivoType::Zanahoria;

	// Estado inicial
	bIsGrabbed = false;
	bWasPlanted = false;
	LastCheckTime = 0.0f;

	// Tag para identificación
	Tags.Add(FName("Seed"));
}

void ASeedItem::BeginPlay()
{
	Super::BeginPlay();

	// Conectar eventos de agarre
	if (GrabComponent)
	{
		GrabComponent->OnToolGrabbed.AddDynamic(this, &ASeedItem::OnGrabbed);
		GrabComponent->OnToolReleased.AddDynamic(this, &ASeedItem::OnReleased);
	}

	UE_LOG(LogTemp, Log, TEXT("SeedItem: %s ready - Type: %s"), 
		*GetName(), 
		*UEnum::GetValueAsString(CultivoType));
}

void ASeedItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Solo verificar cuando está siendo soltada o está cerca del suelo
	if (bWasPlanted)
	{
		return; // Ya fue plantada, no hacer nada más
	}

	// Verificar continuamente si está sobre terreno plantable
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastCheckTime < 0.2f) // Cada 0.2 segundos
	{
		return;
	}

	LastCheckTime = CurrentTime;

	// Si está cerca del suelo, verificar
	if (IsAtPlantingHeight())
	{
		CheckForPlantableGround();
	}
}

// ============================================================
// GRAB EVENTS
// ============================================================

void ASeedItem::OnGrabbed()
{
	bIsGrabbed = true;
	UE_LOG(LogTemp, Log, TEXT("SeedItem: Grabbed - %s"), *GetName());
}

void ASeedItem::OnReleased()
{
	bIsGrabbed = false;
	UE_LOG(LogTemp, Log, TEXT("SeedItem: Released - checking for plantable ground..."));
	
	// Al soltar, verificar inmediatamente si puede plantar
	CheckForPlantableGround();
}

// ============================================================
// PLANTING LOGIC
// ============================================================

void ASeedItem::CheckForPlantableGround()
{
	if (bWasPlanted)
	{
		return;
	}

	// Buscar parcelas cercanas
	TArray<AActor*> FoundParcelas;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(), 
		AParcelaTierra::StaticClass(), 
		FoundParcelas
	);

	if (FoundParcelas.Num() == 0)
	{
		UE_LOG(LogTemp, Verbose, TEXT("SeedItem: No parcelas found in level"));
		return;
	}

	FVector SeedLocation = GetActorLocation();

	// Verificar cada parcela
	for (AActor* Actor : FoundParcelas)
	{
		AParcelaTierra* Parcela = Cast<AParcelaTierra>(Actor);
		if (!Parcela)
		{
			continue;
		}

		// Calcular distancia
		float Distance = FVector::Dist(SeedLocation, Parcela->GetActorLocation());

		// Debug visual
		FColor DebugColor = FColor::Red;
		if (Distance < PlantingRadius && Parcela->CanPlant())
		{
			DebugColor = FColor::Green;
		}

		// Si está lo suficientemente cerca Y la parcela puede plantar
		if (Distance < PlantingRadius && Parcela->CanPlant())
		{
			UE_LOG(LogTemp, Warning, TEXT("SeedItem: Parcela plantable encontrada! Distance: %.2f"), Distance);
			
			// Intentar plantar
			if (TryPlantOnParcela(Parcela))
			{
				return; // Éxito, terminar
			}
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("SeedItem: No plantable parcela nearby"));
}

bool ASeedItem::TryPlantOnParcela(AParcelaTierra* Parcela)
{
	if (!Parcela || !CultivoClass)
	{
		UE_LOG(LogTemp, Error, TEXT("SeedItem: Missing parcela or cultivo class!"));
		return false;
	}

	if (bWasPlanted)
	{
		UE_LOG(LogTemp, Warning, TEXT("SeedItem: Already planted!"));
		return false;
	}

	// Verificar si tiene semillas en inventario (opcional)
	AHarvestHavenGameManager* GameManager = AHarvestHavenGameManager::GetGameManager(this);
	if (GameManager)
	{
		// Si quieres que consuma del inventario:
		/*
		if (!GameManager->HasSeeds(CultivoType, 1))
		{
			UE_LOG(LogTemp, Warning, TEXT("SeedItem: No seeds in inventory!"));
			return false;
		}
		GameManager->RemoveSeeds(CultivoType, 1);
		*/
	}

	// ¡PLANTAR!
	bool bSuccess = Parcela->PlantCrop(CultivoClass, CultivoType);

	if (bSuccess)
	{
		bWasPlanted = true;

		UE_LOG(LogTemp, Error, TEXT("========================================"));
		UE_LOG(LogTemp, Error, TEXT("🌱 ¡SEMILLA PLANTADA EXITOSAMENTE!"));
		UE_LOG(LogTemp, Error, TEXT("Tipo: %s"), *UEnum::GetValueAsString(CultivoType));
		UE_LOG(LogTemp, Error, TEXT("Parcela: %s"), *Parcela->GetName());
		UE_LOG(LogTemp, Error, TEXT("========================================"));
		

		// Destruir la semilla después de plantarla
		SetLifeSpan(0.5f); // Desaparece después de 0.5s

		// Aquí puedes agregar:
		// - Partículas de plantación
		// - Sonido
		// - Haptic feedback

		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("SeedItem: Failed to plant on parcela"));
	return false;
}

bool ASeedItem::IsAtPlantingHeight() const
{
	if (!GetWorld())
	{
		return false;
	}

	// Hacer raycast hacia abajo para verificar altura sobre el suelo
	FVector Start = GetActorLocation();
	FVector End = Start + FVector(0, 0, -500.0f); // 5 metros hacia abajo

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		float HeightAboveGround = FVector::Dist(Start, HitResult.Location);

		return HeightAboveGround < MaxPlantingHeight;
	}

	return false; // No hay suelo debajo
}