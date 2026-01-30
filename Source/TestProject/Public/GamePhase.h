#pragma once

#include "CoreMinimal.h"
#include "GamePhase.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EGamePhase: uint8
{
    WaitingToStart UMETA(DisplayName = "WaitingToStart"),
    InProgress     UMETA(DisplayName = "InProgress"),
    Won            UMETA(DisplayName = "Won"),
    Lost           UMETA(DisplayName = "Lost")
};