// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include <Kismet/GameplayStatics.h>
#include "Enemy.h"
#include "TestProjectGameState.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	GameState = GetWorld()->GetGameState<ATestProjectGameState>();
	GameMode = GetWorld()->GetAuthGameMode<ATestProjectGameMode>();
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

int32 AEnemySpawner::SpawnEnemies()
{
	if (!HasAuthority())
	{
		return 0;
	}
	else 
	{
		// Implementation for spawning enemies would go here
		int32 NumberOfEnemiesToSpawn;
		TSubclassOf<class AEnemy> EnemyClass = AEnemy::StaticClass();
		// 紧急修补，防止 GameMode 未设置导致崩溃
		if (IsValid(GameMode))
		{
			EnemyClass = GameMode->GetEnemyClass();
			if (!EnemyClass)
			{
				return -1;
			}

			NumberOfEnemiesToSpawn = GameMode->GetNumberOfEnemiesToSpawn();
		}
		for (int32 i = 0; i < NumberOfEnemiesToSpawn; ++i)
		{
			FVector SpawnLocation = FVector((i + 1) * 440.0f, 285.0f, 480.0f); // Example spawn location
			FRotator SpawnRotation = FRotator::ZeroRotator;
			AEnemy* SpawnEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyClass, SpawnLocation, SpawnRotation);

			if (GameState)
			{
				SpawnEnemy->OnEnemyDied.AddDynamic(GameState, &ATestProjectGameState::HandleEnemyDied);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Spawned %d enemies"), NumberOfEnemiesToSpawn);

		return NumberOfEnemiesToSpawn;
	}
}