#include "HarvestHavenGameManager.h"
#include "Kismet/GameplayStatics.h"

AHarvestHavenGameManager::AHarvestHavenGameManager()
{
	// Configurar valores por defecto
	PlayerMoney = 0;
	PlayerLevel = 1;
	MoneyForLevel2 = 500;
	MoneyForLevel3 = 1500;
	InitialZanahoriaSeeds = 5;
}

void AHarvestHavenGameManager::BeginPlay()
{
	Super::BeginPlay();

	// Inicializar base de datos si está vacía
	if (CropDatabase.Num() == 0)
	{
		InitializeDefaultCropDatabase();
	}

	// Inicializar datos del jugador
	InitializePlayerData();

	UE_LOG(LogTemp, Warning, TEXT("=== HARVEST HAVEN GAME MANAGER INITIALIZED ==="));
	UE_LOG(LogTemp, Warning, TEXT("Player Money: %d"), PlayerMoney);
	UE_LOG(LogTemp, Warning, TEXT("Player Level: %d"), PlayerLevel);
	UE_LOG(LogTemp, Warning, TEXT("Zanahoria Seeds: %d"), GetSeedCount(ECultivoType::Zanahoria));
}

// ===== INICIALIZACIÓN =====

void AHarvestHavenGameManager::InitializePlayerData()
{
	// Resetear estado del jugador
	PlayerMoney = 0;
	PlayerLevel = 1;
	SeedInventory.Empty();

	// Dar semillas iniciales de zanahoria
	AddSeeds(ECultivoType::Zanahoria, InitialZanahoriaSeeds);

	UE_LOG(LogTemp, Log, TEXT("GameManager: Player data initialized - %d Zanahoria seeds given"), InitialZanahoriaSeeds);
}

void AHarvestHavenGameManager::InitializeDefaultCropDatabase()
{
	// ZANAHORIA - Nivel 1
	FCropInfo Zanahoria;
	Zanahoria.CropType = ECultivoType::Zanahoria;
	Zanahoria.CropName = TEXT("Zanahoria");
	Zanahoria.GrowthTimeSeconds = 120;
	Zanahoria.SellPrice = 20;
	Zanahoria.SeedCost = 10;
	Zanahoria.RequiredLevel = 1;
	CropDatabase.Add(Zanahoria);

	// TOMATE - Nivel 1
	FCropInfo Tomate;
	Tomate.CropType = ECultivoType::Tomate;
	Tomate.CropName = TEXT("Tomate");
	Tomate.GrowthTimeSeconds = 180;
	Tomate.SellPrice = 30;
	Tomate.SeedCost = 15;
	Tomate.RequiredLevel = 1;
	CropDatabase.Add(Tomate);

	// CALABAZA - Nivel 2
	FCropInfo Calabaza;
	Calabaza.CropType = ECultivoType::Calabaza;
	Calabaza.CropName = TEXT("Calabaza");
	Calabaza.GrowthTimeSeconds = 240;
	Calabaza.SellPrice = 50;
	Calabaza.SeedCost = 25;
	Calabaza.RequiredLevel = 2;
	CropDatabase.Add(Calabaza);

	// MAÍZ - Nivel 2
	FCropInfo Maiz;
	Maiz.CropType = ECultivoType::Maiz;
	Maiz.CropName = TEXT("Maíz");
	Maiz.GrowthTimeSeconds = 200;
	Maiz.SellPrice = 40;
	Maiz.SeedCost = 20;
	Maiz.RequiredLevel = 2;
	CropDatabase.Add(Maiz);

	// PLANTA EXÓTICA - Nivel 3
	FCropInfo Exotico;
	Exotico.CropType = ECultivoType::Exotico;
	Exotico.CropName = TEXT("Planta Exótica");
	Exotico.GrowthTimeSeconds = 360;
	Exotico.SellPrice = 100;
	Exotico.SeedCost = 50;
	Exotico.RequiredLevel = 3;
	CropDatabase.Add(Exotico);

	UE_LOG(LogTemp, Log, TEXT("GameManager: Default crop database initialized with %d crops"), CropDatabase.Num());
}

// ===== SISTEMA DE DINERO =====

void AHarvestHavenGameManager::AddMoney(int32 Amount)
{
	int32 OldMoney = PlayerMoney;
	PlayerMoney += Amount;

	// No permitir dinero negativo
	if (PlayerMoney < 0)
	{
		PlayerMoney = 0;
	}

	// Broadcast evento
	OnMoneyChanged.Broadcast(PlayerMoney);

	// Verificar si subió de nivel
	CheckLevelProgression();

	UE_LOG(LogTemp, Log, TEXT("GameManager: Money changed %d -> %d (Amount: %+d)"), 
		OldMoney, PlayerMoney, Amount);
}

bool AHarvestHavenGameManager::HasEnoughMoney(int32 RequiredAmount) const
{
	return PlayerMoney >= RequiredAmount;
}

// ===== SISTEMA DE NIVEL =====

void AHarvestHavenGameManager::CheckLevelProgression()
{
	int32 OldLevel = PlayerLevel;

	// Verificar progresión de nivel
	if (PlayerMoney >= MoneyForLevel3 && PlayerLevel < 3)
	{
		PlayerLevel = 3;
	}
	else if (PlayerMoney >= MoneyForLevel2 && PlayerLevel < 2)
	{
		PlayerLevel = 2;
	}

	// Si subió de nivel, notificar
	if (OldLevel != PlayerLevel)
	{
		OnLevelChanged.Broadcast(PlayerLevel);
		UE_LOG(LogTemp, Warning, TEXT("GameManager: LEVEL UP! %d -> %d"), OldLevel, PlayerLevel);
	}
}

bool AHarvestHavenGameManager::IsCropUnlocked(ECultivoType CropType) const
{
	FCropInfo Info = GetCropInfo(CropType);
	return PlayerLevel >= Info.RequiredLevel;
}

// ===== SISTEMA DE INVENTARIO =====

void AHarvestHavenGameManager::AddSeeds(ECultivoType CropType, int32 Quantity)
{
	if (Quantity <= 0)
		return;

	// Añadir al inventario (si no existe, inicializa en 0)
	if (!SeedInventory.Contains(CropType))
	{
		SeedInventory.Add(CropType, 0);
	}

	SeedInventory[CropType] += Quantity;

	// Broadcast evento
	OnInventoryChanged.Broadcast(CropType, SeedInventory[CropType]);

	UE_LOG(LogTemp, Log, TEXT("GameManager: Added %d seeds of type %d. Total: %d"), 
		Quantity, (int32)CropType, SeedInventory[CropType]);
}

bool AHarvestHavenGameManager::RemoveSeeds(ECultivoType CropType, int32 Quantity)
{
	if (Quantity <= 0)
		return false;

	// Verificar si tiene suficientes semillas
	if (!HasSeeds(CropType, Quantity))
	{
		UE_LOG(LogTemp, Warning, TEXT("GameManager: Not enough seeds of type %d (has %d, needs %d)"), 
			(int32)CropType, GetSeedCount(CropType), Quantity);
		return false;
	}

	// Remover semillas
	SeedInventory[CropType] -= Quantity;

	// Broadcast evento
	OnInventoryChanged.Broadcast(CropType, SeedInventory[CropType]);

	UE_LOG(LogTemp, Log, TEXT("GameManager: Removed %d seeds of type %d. Remaining: %d"), 
		Quantity, (int32)CropType, SeedInventory[CropType]);

	return true;
}

int32 AHarvestHavenGameManager::GetSeedCount(ECultivoType CropType) const
{
	if (SeedInventory.Contains(CropType))
	{
		return SeedInventory[CropType];
	}
	return 0;
}

bool AHarvestHavenGameManager::HasSeeds(ECultivoType CropType, int32 Quantity) const
{
	return GetSeedCount(CropType) >= Quantity;
}

// ===== SISTEMA DE TIENDA =====

bool AHarvestHavenGameManager::BuySeeds(ECultivoType CropType, int32 Quantity)
{
	if (Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameManager: Invalid quantity for BuySeeds: %d"), Quantity);
		return false;
	}

	// Verificar si el cultivo está desbloqueado
	if (!IsCropUnlocked(CropType))
	{
		FCropInfo Info = GetCropInfo(CropType);
		UE_LOG(LogTemp, Warning, TEXT("GameManager: Crop %s is locked (requires level %d)"), 
			*Info.CropName, Info.RequiredLevel);
		return false;
	}

	// Calcular costo total
	int32 TotalCost = GetSeedCost(CropType) * Quantity;

	// Verificar si tiene suficiente dinero
	if (!HasEnoughMoney(TotalCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("GameManager: Not enough money to buy %d seeds (has %d, needs %d)"), 
			Quantity, PlayerMoney, TotalCost);
		return false;
	}

	// Realizar compra
	AddMoney(-TotalCost);
	AddSeeds(CropType, Quantity);

	FCropInfo Info = GetCropInfo(CropType);
	UE_LOG(LogTemp, Log, TEXT("GameManager: Bought %d %s seeds for %d coins"), 
		Quantity, *Info.CropName, TotalCost);

	return true;
}

int32 AHarvestHavenGameManager::SellCrop(ECultivoType CropType, int32 Quantity, bool bIsDried)
{
	if (Quantity <= 0)
		return 0;

	// Calcular precio de venta (50% si está seco)
	int32 PricePerCrop = GetCropSellPrice(CropType, bIsDried);
	int32 TotalEarnings = PricePerCrop * Quantity;

	// Añadir dinero
	AddMoney(TotalEarnings);

	FCropInfo Info = GetCropInfo(CropType);
	UE_LOG(LogTemp, Log, TEXT("GameManager: Sold %d %s for %d coins%s"), 
		Quantity, *Info.CropName, TotalEarnings, bIsDried ? TEXT(" (DRIED)") : TEXT(""));

	return TotalEarnings;
}

// ===== INFORMACIÓN DE CULTIVOS =====

FCropInfo AHarvestHavenGameManager::GetCropInfo(ECultivoType CropType) const
{
	// Buscar en la base de datos
	for (const FCropInfo& Info : CropDatabase)
	{
		if (Info.CropType == CropType)
		{
			return Info;
		}
	}

	// Si no se encuentra, retornar info por defecto
	UE_LOG(LogTemp, Warning, TEXT("GameManager: Crop info not found for type %d, returning default"), (int32)CropType);
	return FCropInfo();
}

int32 AHarvestHavenGameManager::GetCropSellPrice(ECultivoType CropType, bool bIsDried) const
{
	FCropInfo Info = GetCropInfo(CropType);
	int32 BasePrice = Info.SellPrice;

	// Si está seco, vale 50% menos
	if (bIsDried)
	{
		BasePrice = BasePrice / 2;
	}

	return BasePrice;
}

int32 AHarvestHavenGameManager::GetSeedCost(ECultivoType CropType) const
{
	FCropInfo Info = GetCropInfo(CropType);
	return Info.SeedCost;
}

int32 AHarvestHavenGameManager::GetGrowthTime(ECultivoType CropType) const
{
	FCropInfo Info = GetCropInfo(CropType);
	return Info.GrowthTimeSeconds;
}

// ===== UTILIDADES =====

AHarvestHavenGameManager* AHarvestHavenGameManager::GetGameManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
		return nullptr;

	// Obtener GameMode y castearlo
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(WorldContextObject);
	return Cast<AHarvestHavenGameManager>(GameMode);
}