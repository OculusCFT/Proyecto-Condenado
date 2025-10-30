#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyProject/VR/Gameplay/GameplayTypes.h"  // ← AGREGAR ESTA LÍNEA
#include "HarvestHavenGameManager.generated.h"	

// Información de cada tipo de cultivo
USTRUCT(BlueprintType)
struct FCultivoInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECultivoType CropType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CropName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GrowthTimeSeconds = 120; // Tiempo de crecimiento en segundos

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SellPrice = 20; // Precio de venta

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeedCost = 10; // Costo de la semilla

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1; // Nivel requerido para desbloquear

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> CropActorClass; // Clase del actor cultivo

	FCultivoInfo()
		: CropType(ECultivoType::Zanahoria)
		, CropName(TEXT("Zanahoria"))
		, GrowthTimeSeconds(120)
		, SellPrice(20)
		, SeedCost(10)
		, RequiredLevel(1)
		, CropActorClass(nullptr)
	{}
};

// Eventos para notificar cambios en el juego
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoneyChanged, int32, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, ECultivoType, CropType, int32, NewQuantity);

/**
 * GameManager principal de Harvest Haven VR
 * 
 * Responsabilidades:
 * - Gestionar dinero del jugador
 * - Gestionar nivel y progresión (1-3)
 * - Gestionar inventario de semillas
 * - Desbloquear cultivos según nivel
 * - Validar transacciones de compra/venta
 * - Verificar condiciones de victoria
 */
UCLASS()
class MYPROJECT_API AHarvestHavenGameManager : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHarvestHavenGameManager();

	// Eventos broadcasted cuando cambian valores
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnMoneyChanged OnMoneyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnLevelChanged OnLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnInventoryChanged OnInventoryChanged;

protected:
	// ===== CONFIGURACIÓN DE CULTIVOS =====
	
	// Información de todos los cultivos del juego (configurar en Blueprint)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Config|Crops")
	TArray<FCultivoInfo> CropDatabase;

	// ===== PROGRESIÓN DEL JUGADOR =====
	
	// Dinero actual del jugador
	UPROPERTY(BlueprintReadOnly, Category = "Player State")
	int32 PlayerMoney = 0;

	// Nivel actual del jugador (1-3)
	UPROPERTY(BlueprintReadOnly, Category = "Player State")
	int32 PlayerLevel = 1;

	// Inventario de semillas: <TipoCultivo, Cantidad>
	UPROPERTY(BlueprintReadOnly, Category = "Player State")
	TMap<ECultivoType, int32> SeedInventory;

	// ===== CONFIGURACIÓN DE PROGRESIÓN =====
	
	// Dinero necesario para alcanzar el Nivel 2
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Config|Progression")
	int32 MoneyForLevel2 = 500;

	// Dinero necesario para alcanzar el Nivel 3
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Config|Progression")
	int32 MoneyForLevel3 = 1500;

	// Semillas iniciales de zanahoria
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Config|Starting Items")
	int32 InitialZanahoriaSeeds = 5;
	
public:
	virtual void BeginPlay() override;

	// ===== SISTEMA DE DINERO =====
	
	// Añadir dinero al jugador (puede ser negativo para restar)
	UFUNCTION(BlueprintCallable, Category = "Game Manager|Money")
	void AddMoney(int32 Amount);

	// Obtener dinero actual
	UFUNCTION(BlueprintPure, Category = "Game Manager|Money")
	int32 GetPlayerMoney() const { return PlayerMoney; }

	// Verificar si el jugador tiene suficiente dinero
	UFUNCTION(BlueprintPure, Category = "Game Manager|Money")
	bool HasEnoughMoney(int32 RequiredAmount) const;

	// ===== SISTEMA DE NIVEL =====
	
	// Obtener nivel actual
	UFUNCTION(BlueprintPure, Category = "Game Manager|Level")
	int32 GetPlayerLevel() const { return PlayerLevel; }

	// Verificar si un cultivo está desbloqueado
	UFUNCTION(BlueprintPure, Category = "Game Manager|Level")
	bool IsCropUnlocked(ECultivoType CropType) const;

	// ===== SISTEMA DE INVENTARIO =====
	
	// Añadir semillas al inventario
	UFUNCTION(BlueprintCallable, Category = "Game Manager|Inventory")
	void AddSeeds(ECultivoType CropType, int32 Quantity);

	// Remover semillas del inventario (retorna true si había suficientes)
	UFUNCTION(BlueprintCallable, Category = "Game Manager|Inventory")
	bool RemoveSeeds(ECultivoType CropType, int32 Quantity);

	// Obtener cantidad de semillas de un tipo
	UFUNCTION(BlueprintPure, Category = "Game Manager|Inventory")
	int32 GetSeedCount(ECultivoType CropType) const;

	// Verificar si tiene semillas disponibles
	UFUNCTION(BlueprintPure, Category = "Game Manager|Inventory")
	bool HasSeeds(ECultivoType CropType, int32 Quantity = 1) const;

	// ===== SISTEMA DE TIENDA =====
	
	// Comprar semillas (retorna true si fue exitoso)
	UFUNCTION(BlueprintCallable, Category = "Game Manager|Shop")
	bool BuySeeds(ECultivoType CropType, int32 Quantity);

	// Vender cosecha (retorna dinero ganado)
	UFUNCTION(BlueprintCallable, Category = "Game Manager|Shop")
	int32 SellCrop(ECultivoType CropType, int32 Quantity, bool bIsDried = false);

	// ===== INFORMACIÓN DE CULTIVOS =====
	
	// Obtener información completa de un cultivo
	UFUNCTION(BlueprintPure, Category = "Game Manager|Crops")
	FCultivoInfo GetCropInfo(ECultivoType CropType) const;

	// Obtener precio de venta de un cultivo
	UFUNCTION(BlueprintPure, Category = "Game Manager|Crops")
	int32 GetCropSellPrice(ECultivoType CropType, bool bIsDried = false) const;

	// Obtener costo de compra de semilla
	UFUNCTION(BlueprintPure, Category = "Game Manager|Crops")
	int32 GetSeedCost(ECultivoType CropType) const;

	// Obtener tiempo de crecimiento
	UFUNCTION(BlueprintPure, Category = "Game Manager|Crops")
	int32 GetGrowthTime(ECultivoType CropType) const;

	// ===== UTILIDADES =====
	
	// Obtener instancia del GameManager desde cualquier lugar
	UFUNCTION(BlueprintPure, Category = "Game Manager", meta = (WorldContext = "WorldContextObject"))
	static AHarvestHavenGameManager* GetGameManager(const UObject* WorldContextObject);

private:
	// Inicializar datos del jugador (llamado en BeginPlay)
	void InitializePlayerData();

	// Verificar y actualizar nivel del jugador
	void CheckLevelProgression();

	// Inicializar base de datos de cultivos con valores por defecto
	void InitializeDefaultCropDatabase();
};