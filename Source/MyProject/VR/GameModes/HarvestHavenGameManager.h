#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyProject/VR/Gameplay/GameplayTypes.h"
#include "HarvestHavenGameManager.generated.h"	

// Información de cada tipo de cultivo
USTRUCT(BlueprintType)
struct FCropInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECultivoType CropType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CropName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GrowthTimeSeconds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SellPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeedCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> CropActorClass;

	FCropInfo()
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Config|Crops")
	TArray<FCropInfo> CropDatabase;

	// ===== PROGRESIÓN DEL JUGADOR =====
	
	UPROPERTY(BlueprintReadOnly, Category = "Player State")
	int32 PlayerMoney;

	UPROPERTY(BlueprintReadOnly, Category = "Player State")
	int32 PlayerLevel;

	UPROPERTY(BlueprintReadOnly, Category = "Player State")
	TMap<ECultivoType, int32> SeedInventory;

	// ===== CONFIGURACIÓN DE PROGRESIÓN =====
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Config|Progression")
	int32 MoneyForLevel2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Config|Progression")
	int32 MoneyForLevel3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Config|Starting Items")
	int32 InitialZanahoriaSeeds;
	
public:
	virtual void BeginPlay() override;

	// ===== SISTEMA DE DINERO =====
	
	UFUNCTION(BlueprintCallable, Category = "Game Manager|Money")
	void AddMoney(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Game Manager|Money")
	int32 GetPlayerMoney() const { return PlayerMoney; }

	UFUNCTION(BlueprintPure, Category = "Game Manager|Money")
	bool HasEnoughMoney(int32 RequiredAmount) const;

	// ===== SISTEMA DE NIVEL =====
	
	UFUNCTION(BlueprintPure, Category = "Game Manager|Level")
	int32 GetPlayerLevel() const { return PlayerLevel; }

	UFUNCTION(BlueprintPure, Category = "Game Manager|Level")
	bool IsCropUnlocked(ECultivoType CropType) const;

	// ===== SISTEMA DE INVENTARIO =====
	
	UFUNCTION(BlueprintCallable, Category = "Game Manager|Inventory")
	void AddSeeds(ECultivoType CropType, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Game Manager|Inventory")
	bool RemoveSeeds(ECultivoType CropType, int32 Quantity);

	UFUNCTION(BlueprintPure, Category = "Game Manager|Inventory")
	int32 GetSeedCount(ECultivoType CropType) const;

	UFUNCTION(BlueprintPure, Category = "Game Manager|Inventory")
	bool HasSeeds(ECultivoType CropType, int32 Quantity = 1) const;

	// ===== SISTEMA DE TIENDA =====
	
	UFUNCTION(BlueprintCallable, Category = "Game Manager|Shop")
	bool BuySeeds(ECultivoType CropType, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Game Manager|Shop")
	int32 SellCrop(ECultivoType CropType, int32 Quantity, bool bIsDried = false);

	// ===== INFORMACIÓN DE CULTIVOS =====
	
	UFUNCTION(BlueprintPure, Category = "Game Manager|Crops")
	FCropInfo GetCropInfo(ECultivoType CropType) const;

	UFUNCTION(BlueprintPure, Category = "Game Manager|Crops")
	int32 GetCropSellPrice(ECultivoType CropType, bool bIsDried = false) const;

	UFUNCTION(BlueprintPure, Category = "Game Manager|Crops")
	int32 GetSeedCost(ECultivoType CropType) const;

	UFUNCTION(BlueprintPure, Category = "Game Manager|Crops")
	int32 GetGrowthTime(ECultivoType CropType) const;

	// ===== UTILIDADES =====
	
	UFUNCTION(BlueprintPure, Category = "Game Manager", meta = (WorldContext = "WorldContextObject"))
	static AHarvestHavenGameManager* GetGameManager(const UObject* WorldContextObject);

private:
	void InitializePlayerData();
	void CheckLevelProgression();
	void InitializeDefaultCropDatabase();
};