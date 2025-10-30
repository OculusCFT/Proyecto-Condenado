#include "Cultivo.h"
#include "Kismet/GameplayStatics.h"
#include "MyProject/VR/GameModes/HarvestHavenGameManager.h"

ACultivo::ACultivo()
{
	PrimaryActorTick.bCanEverTick = true;

	// Crear mesh component
	CultivoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CultivoMesh"));
	RootComponent = CultivoMesh;
	CultivoMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Estado inicial
	CurrentState = ECultivoState::Semilla;
	TiempoTranscurrido = 0.0f;
	TiempoUltimoRiego = 0.0f;
	bNecesitaRiego = false;
	bFueCosechado = false;

	// Tag para identificación
	Tags.Add(FName("Cultivo"));
}

void ACultivo::BeginPlay()
{
	Super::BeginPlay();

	// Configurar mesh inicial
	UpdateVisualMesh();

	// Obtener configuración del GameManager automáticamente
	if (AHarvestHavenGameManager* GameManager = Cast<AHarvestHavenGameManager>(
		UGameplayStatics::GetGameMode(this)))
	{
		FCultivoInfo Info = GameManager->GetCultivoInfo(TipoCultivo);
		TiempoCrecimientoSegundos = Info.TiempoCrecimientoSegundos;
		ValorCosecha = Info.ValorCosecha;
	}

	// Registrar tiempo inicial de riego
	TiempoUltimoRiego = GetWorld()->GetTimeSeconds();

	UE_LOG(LogTemp, Log, TEXT("Cultivo: %s planted - Growth time: %.1fs"), 
		*GetName(), TiempoCrecimientoSegundos);
}

void ACultivo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// No actualizar si ya fue cosechado
	if (bFueCosechado)
	{
		return;
	}

	// Actualizar crecimiento
	UpdateGrowthState(DeltaTime);

	// Actualizar necesidad de riego
	UpdateWaterNeed(DeltaTime);
}

// ============================================================
// GROWTH SYSTEM
// ============================================================

void ACultivo::StartGrowth()
{
	TiempoTranscurrido = 0.0f;
	TiempoUltimoRiego = GetWorld()->GetTimeSeconds();
	ChangeState(ECultivoState::Semilla);
	
	UE_LOG(LogTemp, Log, TEXT("Cultivo: Growth started for %s"), *GetName());
}

float ACultivo::GetGrowthPercent() const
{
	if (TiempoCrecimientoSegundos <= 0.0f)
	{
		return 0.0f;
	}

	float Percent = (TiempoTranscurrido / TiempoCrecimientoSegundos) * 100.0f;
	return FMath::Clamp(Percent, 0.0f, 100.0f);
}

void ACultivo::UpdateGrowthState(float DeltaTime)
{
	// Si ya está maduro o seco, no crecer más
	if (CurrentState == ECultivoState::Maduro || CurrentState == ECultivoState::Seco)
	{
		return;
	}

	// Incrementar tiempo
	TiempoTranscurrido += DeltaTime;

	// Calcular porcentaje de crecimiento
	float GrowthPercent = GetGrowthPercent();

	// Determinar estado según porcentaje
	ECultivoState NewState = CurrentState;

	if (GrowthPercent < 33.0f)
	{
		NewState = ECultivoState::Semilla;
	}
	else if (GrowthPercent < 100.0f)
	{
		NewState = ECultivoState::Creciendo;
	}
	else // >= 100%
	{
		NewState = ECultivoState::Maduro;
	}

	// Si cambió de estado, actualizar
	if (NewState != CurrentState)
	{
		ChangeState(NewState);
	}
}

void ACultivo::ChangeState(ECultivoState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}

	ECultivoState OldState = CurrentState;
	CurrentState = NewState;

	// Actualizar visual
	UpdateVisualMesh();

	// Broadcast evento
	OnStateChanged.Broadcast(CurrentState, GetGrowthPercent());

	UE_LOG(LogTemp, Log, TEXT("Cultivo: State changed %s -> %s (%.1f%%)"), 
		*UEnum::GetValueAsString(OldState), 
		*UEnum::GetValueAsString(CurrentState),
		GetGrowthPercent());
}

// ============================================================
// WATERING SYSTEM
// ============================================================

void ACultivo::Water()
{
	// No se puede regar si está maduro o seco
	if (CurrentState == ECultivoState::Maduro)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Cultivo: Already mature, doesn't need water"));
		return;
	}

	if (CurrentState == ECultivoState::Seco)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cultivo: Too late, already dry"));
		return;
	}

	// Actualizar tiempo de riego
	TiempoUltimoRiego = GetWorld()->GetTimeSeconds();
	bNecesitaRiego = false;

	UE_LOG(LogTemp, Log, TEXT("Cultivo: Watered successfully"));
}

float ACultivo::GetTimeSinceLastWater() const
{
	if (!GetWorld())
	{
		return 0.0f;
	}

	return GetWorld()->GetTimeSeconds() - TiempoUltimoRiego;
}

void ACultivo::UpdateWaterNeed(float DeltaTime)
{
	// Si ya está maduro o seco, no verificar riego
	if (CurrentState == ECultivoState::Maduro || CurrentState == ECultivoState::Seco)
	{
		return;
	}

	float TimeSinceWater = GetTimeSinceLastWater();

	// Verificar si necesita riego (después de IntervaloRiego segundos)
	if (!bNecesitaRiego && TimeSinceWater >= IntervaloRiego)
	{
		bNecesitaRiego = true;
		OnNeedsWater.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("Cultivo: NEEDS WATER! (%.1fs since last water)"), TimeSinceWater);
	}

	// Verificar si se secó (después de TiempoAntesDeSecar segundos sin riego)
	if (TimeSinceWater >= TiempoAntesDeSecar)
	{
		ChangeState(ECultivoState::Seco);
		UE_LOG(LogTemp, Error, TEXT("Cultivo: DRIED UP! Value reduced to 50%%"));
	}
}

// ============================================================
// HARVEST SYSTEM
// ============================================================

bool ACultivo::TryHarvest(int32& OutValue)
{
	// Solo se puede cosechar si está maduro o seco
	if (CurrentState != ECultivoState::Maduro && CurrentState != ECultivoState::Seco)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cultivo: Cannot harvest - not ready (%.1f%%)"), 
			GetGrowthPercent());
		OutValue = 0;
		return false;
	}

	// Ya fue cosechado
	if (bFueCosechado)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cultivo: Already harvested"));
		OutValue = 0;
		return false;
	}

	// Calcular valor
	OutValue = GetCurrentHarvestValue();
	bFueCosechado = true;

	// Broadcast evento
	OnHarvested.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("Cultivo: Harvested successfully - Value: %d"), OutValue);

	// El actor se puede destruir después o dejar que la cesta lo maneje
	return true;
}

int32 ACultivo::GetCurrentHarvestValue() const
{
	if (CurrentState == ECultivoState::Seco)
	{
		// Cultivos secos valen 50% menos
		return ValorCosecha / 2;
	}

	return ValorCosecha;
}

// ============================================================
// VISUAL SYSTEM
// ============================================================

void ACultivo::UpdateVisualMesh()
{
	if (!CultivoMesh)
	{
		return;
	}

	UStaticMesh* NewMesh = GetMeshForState(CurrentState);
	
	if (NewMesh)
	{
		CultivoMesh->SetStaticMesh(NewMesh);
		UE_LOG(LogTemp, Verbose, TEXT("Cultivo: Visual mesh updated for state %s"), 
			*UEnum::GetValueAsString(CurrentState));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cultivo: No mesh assigned for state %s"), 
			*UEnum::GetValueAsString(CurrentState));
	}
}

UStaticMesh* ACultivo::GetMeshForState(ECultivoState State) const
{
	switch (State)
	{
		case ECultivoState::Semilla:
			return MeshSemilla;
		case ECultivoState::Creciendo:
			return MeshCreciendo;
		case ECultivoState::Maduro:
			return MeshMaduro;
		case ECultivoState::Seco:
			return MeshSeco ? MeshSeco : MeshMaduro; // Fallback a maduro si no hay mesh seco
		default:
			return nullptr;
	}
}