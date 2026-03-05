// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include <Enemy.h>
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Components/StateTreeComponent.h"
#include "Net/UnrealNetwork.h"
#include "StateTree.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Alerted,
	Chasing,
	Attacking,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyAIAttack, AActor*, AttackActor);

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
	UFUNCTION(BlueprintAuthorityOnly, Category = "State")
	void UpdateAlertedState();
	UFUNCTION(BlueprintCallable, Category = "State")
	void EnterAlertedState();


	void ChangeState(EEnemyState NewState);
	void ExitCurrentState();

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
	// 最后已知位置
	FVector LastKnownPlayerLocation;
	// 是否当前可见
	UPROPERTY(VisibleInstanceOnly, Category = "Perception")
	bool bHasLineOfSight = true;
	// 丢失目标时间
	float LostSightTime = 0.f;
	// 允许记忆时间
	float MemoryDuration = 3.f;

	// AI Pathfinding properties
	float ChaseRepathInterval = 0.3f;
	float LastChaseRepathTime = 0.f;

	// AI Alert properties
	float AlertDuration = 5.f;
	float LastAlertRepathTime = 0.f;

	// AI Flank properties
	float FlankSide = 1.f;

public:
	// 受到伤害的反应
	UFUNCTION()
	void OnDamageTaken(AActor* InstigatorActor);
	FOnEnemyAIAttack OnEnemyAIAttack;

protected:
	// State Tree Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UStateTreeComponent> StateTreeComponent;
	// State Tree Asset
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UStateTree> StateTreeAsset;
};
