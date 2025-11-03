// ==================================================================
// WateringCan.cpp
// Implementación de la regadera con sistema de riego y llenado
// ==================================================================

#include "WateringCan.h"
#include "Cultivo.h"
#include "MyProject/VR/Components/VRGrabComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"

AWateringCan::AWateringCan()
{
	PrimaryActorTick.bCanEverTick = true;

	// Cuerpo de la regadera
	CanMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CanMesh"));
	RootComponent = CanMesh;
	CanMesh->SetSimulatePhysics(true);
	CanMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	CanMesh->SetMassOverrideInKg(NAME_None, 1.5f); // Pesada cuando llena

	// Pico/boquilla
	SpoutMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpoutMesh"));
	SpoutMesh->SetupAttachment(CanMesh);
	SpoutMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpoutMesh->SetRelativeLocation(FVector(30.0f, 0.0f, 10.0f));

	// Punto de spawn de agua
	WaterSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("WaterSpawnPoint"));
	WaterSpawnPoint->SetupAttachment(SpoutMesh);
	WaterSpawnPoint->SetRelativeLocation(FVector(10.0f, 0.0f, -5.0f));

	// Grab component
	GrabComponent = CreateDefaultSubobject<UVRGrabComponent>(TEXT("GrabComponent"));
	GrabComponent->GrabType = EGrabType::Attach;

	// Config por defecto
	MaxWaterCapacity = 10.0f; // 10 usos
	WaterPerUse = 1.0f;
	WateringRadius = 80.0f;
	RefillRadius = 150.0f;
	RefillRate = 2.0f; // 2 unidades por segundo
	
	bRequireTilt = true;
	MinTiltAngle = 45.0f; // Debe inclinarse al menos 45 grados
	
	WaterSourceTag = FName("WaterSource");

	// Estado inicial
	CurrentWater = MaxWaterCapacity; // Empieza llena
	bIsGrabbed = false;
	bIsWatering = false;
	bIsRefilling = false;
	LastCheckTime = 0.0f;
	LastWaterTime = 0.0f;
	LastWateredCrop = nullptr;

	// Tag
	Tags.Add(FName("WateringCan"));
}

void AWateringCan::BeginPlay()
{
	Super::BeginPlay();

	// Conectar eventos
	if (GrabComponent)
	{
		GrabComponent->OnToolGrabbed.AddDynamic(this, &AWateringCan::OnGrabbed);
		GrabComponent->OnToolReleased.AddDynamic(this, &AWateringCan::OnReleased);
	}

	UE_LOG(LogTemp, Warning, TEXT("WateringCan: Ready - Water: %.1f/%.1f"), 
		CurrentWater, MaxWaterCapacity);
}

void AWateringCan::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsGrabbed)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Verificar llenado (cuando está en agua)
	if (CurrentTime - LastCheckTime > 0.1f)
	{
		CheckForWaterSource();
		LastCheckTime = CurrentTime;
	}

	// Si está llenándose, llenar gradualmente
	if (bIsRefilling && !IsFull())
	{
		RefillWater(DeltaTime);
	}

	// Verificar si está regando
	if (!IsEmpty() && IsProperlyTilted())
	{
		// Verificar cada 0.3 segundos para regar
		if (CurrentTime - LastCheckTime > 0.3f)
		{
			CheckForCropsToWater();
			LastCheckTime = CurrentTime;
		}

		if (!bIsWatering)
		{
			StartWateringEffects();
		}
	}
	else
	{
		if (bIsWatering)
		{
			StopWateringEffects();
		}
	}
}

// ============================================================
// GRAB EVENTS
// ============================================================

void AWateringCan::OnGrabbed()
{
	bIsGrabbed = true;
	UE_LOG(LogTemp, Log, TEXT("WateringCan: Grabbed - Water: %.1f%%"), 
		GetWaterPercentage() * 100.0f);
}

void AWateringCan::OnReleased()
{
	bIsGrabbed = false;
	bIsRefilling = false;
	
	if (bIsWatering)
	{
		StopWateringEffects();
	}
	
	UE_LOG(LogTemp, Log, TEXT("WateringCan: Released"));
}

// ============================================================
// WATERING LOGIC
// ============================================================

void AWateringCan::CheckForCropsToWater()
{
	if (IsEmpty())
	{
		return;
	}

	// Buscar cultivos cercanos
	TArray<AActor*> FoundCultivos;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(), 
		ACultivo::StaticClass(), 
		FoundCultivos
	);

	if (FoundCultivos.Num() == 0)
	{
		return;
	}

	FVector SpoutLocation = WaterSpawnPoint->GetComponentLocation();
	

	// Verificar cada cultivo
	for (AActor* Actor : FoundCultivos)
	{
		ACultivo* Cultivo = Cast<ACultivo>(Actor);
		if (!Cultivo)
		{
			continue;
		}

		float Distance = FVector::Dist(SpoutLocation, Cultivo->GetActorLocation());

		// Si está cerca, regarlo
		if (Distance < WateringRadius)
		{
			WaterCrop(Cultivo);
			return; // Solo regar uno a la vez
		}
	}
}

bool AWateringCan::WaterCrop(ACultivo* Crop)
{
	if (!Crop || IsEmpty())
	{
		return false;
	}

	// Cooldown para evitar regar el mismo cultivo muy rápido
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (Crop == LastWateredCrop && (CurrentTime - LastWaterTime) < 2.0f)
	{
		return false;
	}

	// Ya está maduro o seco, no necesita agua
	if (Crop->IsMature() || Crop->IsDry())
	{
		UE_LOG(LogTemp, Verbose, TEXT("WateringCan: Crop doesn't need water (mature/dry)"));
		return false;
	}

	// Consumir agua
	if (!ConsumeWater(WaterPerUse))
	{
		UE_LOG(LogTemp, Warning, TEXT("WateringCan: Out of water!"));
		return false;
	}

	// ¡REGAR!
	Crop->Water();

	LastWateredCrop = Crop;
	LastWaterTime = CurrentTime;

	UE_LOG(LogTemp, Warning, TEXT("💧 CULTIVO REGADO! Water left: %.1f/%.1f"), 
		CurrentWater, MaxWaterCapacity);

	// Efecto visual en el cultivo
	DrawDebugSphere(
		GetWorld(),
		Crop->GetActorLocation(),
		50.0f,
		16,
		FColor::Blue,
		false,
		2.0f,
		0,
		3.0f
	);

	return true;
}

// ============================================================
// REFILL LOGIC
// ============================================================

void AWateringCan::CheckForWaterSource()
{
	if (IsFull())
	{
		bIsRefilling = false;
		return;
	}

	// Buscar actores con tag "WaterSource"
	TArray<AActor*> FoundSources;
	UGameplayStatics::GetAllActorsWithTag(
		GetWorld(), 
		WaterSourceTag, 
		FoundSources
	);

	if (FoundSources.Num() == 0)
	{
		bIsRefilling = false;
		return;
	}

	FVector CanLocation = GetActorLocation();

	for (AActor* Source : FoundSources)
	{
		float Distance = FVector::Dist(CanLocation, Source->GetActorLocation());

		// Debug
		DrawDebugLine(
			GetWorld(),
			CanLocation,
			Source->GetActorLocation(),
			Distance < RefillRadius ? FColor::Green : FColor::Yellow,
			false,
			0.3f,
			0,
			2.0f
		);

		if (Distance < RefillRadius)
		{
			if (!bIsRefilling)
			{
				bIsRefilling = true;
				PlayRefillEffects();
				UE_LOG(LogTemp, Warning, TEXT("WateringCan: Started refilling at %s"), 
					*Source->GetName());
			}
			return;
		}
	}

	bIsRefilling = false;
}

void AWateringCan::RefillWater(float DeltaTime)
{
	float AmountToAdd = RefillRate * DeltaTime;
	CurrentWater = FMath::Clamp(CurrentWater + AmountToAdd, 0.0f, MaxWaterCapacity);

	if (IsFull())
	{
		bIsRefilling = false;
		UE_LOG(LogTemp, Warning, TEXT("WateringCan: FULL! 💧"));
	}
}

// ============================================================
// TILT DETECTION
// ============================================================

bool AWateringCan::IsProperlyTilted() const
{
	if (!bRequireTilt)
	{
		return true; // No requiere inclinación
	}

	// Obtener el vector "arriba" de la regadera
	FVector UpVector = GetActorUpVector();
	
	// Comparar con el vector arriba del mundo
	float DotProduct = FVector::DotProduct(UpVector, FVector::UpVector);
	
	// Convertir a ángulo
	float AngleInDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	// Debug visual
	FVector CanLocation = GetActorLocation();
	DrawDebugDirectionalArrow(
		GetWorld(),
		CanLocation,
		CanLocation + (UpVector * 50.0f),
		20.0f,
		AngleInDegrees > MinTiltAngle ? FColor::Green : FColor::Red,
		false,
		0.1f,
		0,
		3.0f
	);

	return AngleInDegrees > MinTiltAngle;
}

// ============================================================
// WATER MANAGEMENT
// ============================================================

bool AWateringCan::ConsumeWater(float Amount)
{
	if (CurrentWater < Amount)
	{
		return false;
	}

	CurrentWater -= Amount;
	return true;
}

float AWateringCan::GetWaterPercentage() const
{
	return CurrentWater / MaxWaterCapacity;
}

bool AWateringCan::HasWater() const
{
	return CurrentWater >= WaterPerUse;
}

bool AWateringCan::IsFull() const
{
	return CurrentWater >= MaxWaterCapacity;
}

bool AWateringCan::IsEmpty() const
{
	return CurrentWater <= 0.0f;
}

// ============================================================
// EFFECTS
// ============================================================

void AWateringCan::StartWateringEffects()
{
	bIsWatering = true;

	// Spawn partículas de agua
	if (WaterParticles && WaterSpawnPoint)
	{
		UGameplayStatics::SpawnEmitterAttached(
			WaterParticles,
			WaterSpawnPoint,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	// Reproducir sonido
	if (WateringSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WateringSound,
			GetActorLocation()
		);
	}

	UE_LOG(LogTemp, Verbose, TEXT("WateringCan: Started watering"));
}

void AWateringCan::StopWateringEffects()
{
	bIsWatering = false;
	UE_LOG(LogTemp, Verbose, TEXT("WateringCan: Stopped watering"));
}

void AWateringCan::PlayRefillEffects()
{
	if (RefillSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			RefillSound,
			GetActorLocation()
		);
	}

	// Efecto visual de llenado
	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		RefillRadius,
		16,
		FColor::Cyan,
		false,
		1.0f,
		0,
		2.0f
	);
}