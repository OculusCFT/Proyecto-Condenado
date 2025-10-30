#pragma once

#include "CoreMinimal.h"
#include "GameplayTypes.generated.h"

/**
 * Tipos de cultivos disponibles en el juego
 */
UENUM(BlueprintType)
enum class ECultivoType : uint8
{
	Zanahoria UMETA(DisplayName = "Zanahoria"),
	Tomate UMETA(DisplayName = "Tomate"),
	Calabaza UMETA(DisplayName = "Calabaza"),
	Maiz UMETA(DisplayName = "Maíz"),
	Exotico UMETA(DisplayName = "Planta Exótica")
};

/**
 * Niveles de progresión del jugador
 */
UENUM(BlueprintType)
enum class EPlayerLevel : uint8
{
	Level1 UMETA(DisplayName = "Nivel 1 - Principiante"),
	Level2 UMETA(DisplayName = "Nivel 2 - Intermedio"),
	Level3 UMETA(DisplayName = "Nivel 3 - Maestro")
};