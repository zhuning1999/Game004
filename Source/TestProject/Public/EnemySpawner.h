// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestProject/TestProjectGameMode.h"
#include "EnemySpawner.generated.h"

class ATestProjectGameState;

UCLASS()
class TESTPROJECT_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	ATestProjectGameMode* GameMode;
	UPROPERTY(EditAnywhere, Category = "Spawning")
	ATestProjectGameState* GameState;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Begain spawn enemies
	UFUNCTION()
	int32 SpawnEnemies();
};
