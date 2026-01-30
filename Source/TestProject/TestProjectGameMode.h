// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GamePhase.h"
#include "TestProjectGameMode.generated.h"

UCLASS(minimalapi)
class ATestProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATestProjectGameMode();

	// Get number of enemies to spawn
	UFUNCTION(BlueprintCallable, Category = "Game|Rule")
	int32 GetNumberOfEnemiesToSpawn() const { return NumberOfEnemiesToSpawn; }
	// Get enemy class to spawn
	UFUNCTION(BlueprintCallable, Category = "Game|Rule")
	TSubclassOf<class AEnemy> GetEnemyClass() const { return EnemyClass; }

protected:
	virtual void BeginPlay() override;

protected:
	// Enemy class to spawn
	UPROPERTY(EditDefaultsonly, Category = "Game|Enemy")
	TSubclassOf<class AEnemy> EnemyClass;
	// Number of enemies to spawn
	UPROPERTY(EditDefaultsonly, Category = "Game|Enemy")
	int32 NumberOfEnemiesToSpawn = 2;
};



