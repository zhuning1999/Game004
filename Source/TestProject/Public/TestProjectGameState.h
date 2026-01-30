// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GamePhase.h"
#include "TestProjectGameState.generated.h"

/**
 * 
 */
class AEnemySpawner;

UCLASS()
class TESTPROJECT_API ATestProjectGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	// By handle enemy died event, controlling the game phase 
	UFUNCTION()
	virtual void HandleEnemyDied();

	// Offer a way to change the game phase
	UFUNCTION(BlueprintCallable, Category = "Game|Phase")
	void SetGamePhase(EGamePhase NewPhase);

protected:
	UPROPERTY(EditAnywhere, Category = "Game|Enemy")
	int32 NumberOfEnemiesToSpawn = 1;
	// Enemy spawner spot
	UPROPERTY()
	AEnemySpawner* SpawnEnemySpot;
	// Current game phase
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Phase")
	EGamePhase CurrentGamePhase = EGamePhase::WaitingToStart;

	// Called when the game phase changes
	UFUNCTION(Category = "Game|Phase")
	void OnGamePhaseChanged(EGamePhase OldPhase, EGamePhase NewPhase);


	virtual void BeginPlay() override;
};
