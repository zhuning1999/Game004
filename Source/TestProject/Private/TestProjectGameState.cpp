// Fill out your copyright notice in the Description page of Project Settings.


#include "TestProjectGameState.h"
#include "EnemySpawner.h"
#include <Kismet/GameplayStatics.h>


void ATestProjectGameState::HandleEnemyDied()
{
	NumberOfEnemiesToSpawn--;
	if (NumberOfEnemiesToSpawn <= 0)
	{
		SetGamePhase(EGamePhase::Won);
	}
}

void ATestProjectGameState::BeginPlay()
{
	Super::BeginPlay();

	// 找场景中第一个 EnemySpawner
	TArray<AActor*> FoundSpawners;
	UGameplayStatics::GetAllActorsOfClass(this, AEnemySpawner::StaticClass(), FoundSpawners);

	if (FoundSpawners.Num() > 0)
	{
		SpawnEnemySpot = Cast<AEnemySpawner>(FoundSpawners[0]);
	}

	SetGamePhase(EGamePhase::InProgress);
}

void ATestProjectGameState::SetGamePhase(EGamePhase NewPhase)
{
	if (NewPhase == CurrentGamePhase)
	{
		return;
	}
	const EGamePhase OldPhase = CurrentGamePhase;
	CurrentGamePhase = NewPhase;

	UE_LOG(LogTemp, Log, TEXT("GamePhase: %s -> %s"),
		*UEnum::GetValueAsString(OldPhase),
		*UEnum::GetValueAsString(NewPhase));

	OnGamePhaseChanged(OldPhase, NewPhase);
}

void ATestProjectGameState::OnGamePhaseChanged(EGamePhase OldPhase, EGamePhase NewPhase)
{
	switch (NewPhase)
	{
	case EGamePhase::WaitingToStart:
		break;
	case EGamePhase::InProgress:
		NumberOfEnemiesToSpawn += SpawnEnemySpot->SpawnEnemies();
		break;
	case EGamePhase::Won:
		UE_LOG(LogTemp, Log, TEXT("You Win!"));
		break;
	case EGamePhase::Lost:
		UE_LOG(LogTemp, Log, TEXT("You Lose!"));
		break;
	default:
		break;
	}
}
