// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include <Enemy.h>
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Chasing,
	Attacking,
	Dead
};

UCLASS()
class TESTPROJECT_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	AEnemyAIController();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

	// Target Pawn
	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	APawn* TargetPawn;

	// Possessed Pawn
	UPROPERTY()
	APawn* PossessedPawn;
	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	AEnemy* EnemyPawn;

	// Enemy state
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	EEnemyState EnemyState = EEnemyState::Idle;

	UPROPERTY(VisibleInstanceOnly, Category = "State")
	float LastAttackTime = 0.0f;

	// In Attack range check
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool IsInAttackRange(APawn* TargetActor) const;

	// In Detect range check
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool IsInDetectRange(APawn* TargetActor) const;

	// State transition functions
	UFUNCTION(BlueprintAuthorityOnly, Category = "State")
	void UpdateCurrentState();
	UFUNCTION(BlueprintAuthorityOnly, Category = "State")
	void UpdateChaseState();
	UFUNCTION(BlueprintCallable, Category = "State")
	void EnterChaseState();
	UFUNCTION(BlueprintAuthorityOnly, Category = "State")
	void UpdateAttackState();
	UFUNCTION(BlueprintCallable, Category = "State")
	void EnterAttackState();
	UFUNCTION(BlueprintAuthorityOnly, Category = "State")
	void UpdateIdleState();
	UFUNCTION(BlueprintCallable, Category = "State")
	void EnterIdleState();

	// Handle possessed pawn destroyed
	UFUNCTION()
	void OnPossessedPawnDestroyed();

	// Handle possessed pawn attack finished
	UFUNCTION()
	void OnPossessedPawnAttackFinished();

	// AI Perception
	// Sight Config
	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;
	// Initialize Perception System
	UFUNCTION()
	void InitializePerceptionSystem();
	UFUNCTION()
	// Setup Perception System
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
