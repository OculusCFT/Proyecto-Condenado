#include "ParcelaTierra.h"
#include "Cultivo.h"
#include "Kismet/GameplayStatics.h"
#include "MyProject/VR/GameModes/HarvestHavenGameManager.h"

AParcelaTierra::AParcelaTierra()
{
	PrimaryActorTick.bCanEverTick = false;

	// Crear mesh de tierra
	TierraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TierraMesh"));
	RootComponent = TierraMesh;
	TierraMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Punto de spawn para cultivo (encima de la tierra)
	CultivoSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CultivoSpawnPoint"));
	CultivoSpawnPoint->SetupAttachment(TierraMesh);
	CultivoSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f)); // 10cm arriba

	// Estado inicial
	CurrentState = EParcelaState::SinPreparar;
	CurrentCultivo = nullptr;

	// Tag para identificación
	Tags.Add(FName("Parcela"));
}

void AParcelaTierra::BeginPlay()
{
	Super::BeginPlay();

	// Configurar mesh inicial
	UpdateVisualMesh();

	UE_LOG(LogTemp, Log, TEXT("ParcelaTierra: Initialized at %s"), 
		*GetActorLocation().ToString());
}

// ============================================================
// PREPARATION SYSTEM
// ============================================================

bool AParcelaTierra::PrepareGround()
{
	// Verificar si se puede preparar
	if (!CanBePrepared())
	{
		UE_LOG(LogTemp, Warning, TEXT("ParcelaTierra: Cannot prepare - State: %s"), 
			*UEnum::GetValueAsString(CurrentState));
		return false;
	}

	// Cambiar a estado preparada
	ChangeState(EParcelaState::Preparada);

	UE_LOG(LogTemp, Log, TEXT("ParcelaTierra: Ground prepared successfully"));
	return true;
}

// ============================================================
// PLANTING SYSTEM
// ============================================================

bool AParcelaTierra::PlantCrop(TSubclassOf<ACultivo> CultivoClass, ECultivoType TipoCultivo)
{
	// Verificar condiciones
	if (!CanPlant())
	{
		UE_LOG(LogTemp, Warning, TEXT("ParcelaTierra: Cannot plant - State: %s"), 
			*UEnum::GetValueAsString(CurrentState));
		return false;
	}

	if (!CultivoClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ParcelaTierra: Cultivo class is null!"));
		return false;
	}

	// Spawn cultivo en el punto designado
	FVector SpawnLocation = GetCultivoSpawnLocation();
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACultivo* NewCultivo = GetWorld()->SpawnActor<ACultivo>(
		CultivoClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!NewCultivo)
	{
		UE_LOG(LogTemp, Error, TEXT("ParcelaTierra: Failed to spawn cultivo!"));
		return false;
	}

	// Configurar cultivo
	NewCultivo->TipoCultivo = TipoCultivo;
	
	// Obtener datos del GameManager si está disponible
	if (AHarvestHavenGameManager* GameManager = Cast<AHarvestHavenGameManager>(
		UGameplayStatics::GetGameMode(this)))
	{
		FCropInfo Info = GameManager->GetCropInfo(TipoCultivo);
		NewCultivo->TiempoCrecimientoSegundos = Info.GrowthTimeSeconds;
		NewCultivo->ValorCosecha = Info.SellPrice;
	}

	// Iniciar crecimiento
	NewCultivo->StartGrowth();

	// Actualizar estado
	CurrentCultivo = NewCultivo;
	ChangeState(EParcelaState::ConCultivo);

	// Broadcast evento
	OnCultivoPlanted.Broadcast(NewCultivo);

	UE_LOG(LogTemp, Log, TEXT("ParcelaTierra: Cultivo planted successfully - Type: %s"), 
		*UEnum::GetValueAsString(TipoCultivo));

	return true;
}

FVector AParcelaTierra::GetCultivoSpawnLocation() const
{
	if (CultivoSpawnPoint)
	{
		return CultivoSpawnPoint->GetComponentLocation();
	}

	// Fallback: spawn un poco arriba del actor
	return GetActorLocation() + FVector(0.0f, 0.0f, 10.0f);
}

// ============================================================
// HARVEST SYSTEM
// ============================================================

bool AParcelaTierra::HarvestCrop(int32& OutValue)
{
	OutValue = 0;

	// Verificar que hay cultivo
	if (!HasCultivo())
	{
		UE_LOG(LogTemp, Warning, TEXT("ParcelaTierra: No crop to harvest"));
		return false;
	}

	// Intentar cosechar
	bool bSuccess = CurrentCultivo->TryHarvest(OutValue);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("ParcelaTierra: Crop harvested - Value: %d"), OutValue);
		
		// Limpiar parcela
		ClearParcela();
		return true;
	}

	return false;
}

void AParcelaTierra::ClearParcela()
{
	// Destruir cultivo si existe
	if (CurrentCultivo && IsValid(CurrentCultivo))
	{
		CurrentCultivo->Destroy();
	}

	CurrentCultivo = nullptr;

	// Volver a estado preparada (mantener el surco)
	ChangeState(EParcelaState::Preparada);

	UE_LOG(LogTemp, Log, TEXT("ParcelaTierra: Cleared - Ready for new crop"));
}

// ============================================================
// STATE MANAGEMENT
// ============================================================

void AParcelaTierra::ChangeState(EParcelaState NewState)
{
	if (NewState == CurrentState)
	{
		return;
	}

	EParcelaState OldState = CurrentState;
	CurrentState = NewState;

	// Actualizar visual
	UpdateVisualMesh();

	// Broadcast evento
	OnStateChanged.Broadcast(CurrentState);

	UE_LOG(LogTemp, Log, TEXT("ParcelaTierra: State changed %s -> %s"), 
		*UEnum::GetValueAsString(OldState), 
		*UEnum::GetValueAsString(CurrentState));
}

// ============================================================
// VISUAL SYSTEM
// ============================================================

void AParcelaTierra::UpdateVisualMesh()
{
	if (!TierraMesh)
	{
		return;
	}

	UStaticMesh* NewMesh = GetMeshForState(CurrentState);
	
	if (NewMesh)
	{
		TierraMesh->SetStaticMesh(NewMesh);
		UE_LOG(LogTemp, Verbose, TEXT("ParcelaTierra: Visual mesh updated for state %s"), 
			*UEnum::GetValueAsString(CurrentState));
	}
}

UStaticMesh* AParcelaTierra::GetMeshForState(EParcelaState State) const
{
	switch (State)
	{
		case EParcelaState::SinPreparar:
			return MeshSinPreparar;
		case EParcelaState::Preparada:
		case EParcelaState::ConCultivo: // Usa mismo mesh preparada cuando tiene cultivo
			return MeshPreparada;
		default:
			return nullptr;
	}
}