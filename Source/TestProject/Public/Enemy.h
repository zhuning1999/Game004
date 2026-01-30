// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "Net/UnrealNetwork.h"
#include "Enemy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyAttackFinished);

UCLASS()
class TESTPROJECT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	FOnEnemyDied OnEnemyDied;
	UPROPERTY(BlueprintAssignable)
	// Finish attack
	FOnEnemyAttackFinished OnEnemyAttackFinished;
	UFUNCTION(BlueprintCallable)
	void EnemyAttackFinishedBroadcast() { OnEnemyAttackFinished.Broadcast(); }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Detect range
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float DetectRange = 1500.0f;
	// Target Pawn
	UPROPERTY(VisibleAnywhere, Category = "Enemy")
	APawn* TargetPawn;
public:
	// Set Target Pawn
	void SetTargetPawn(APawn* NewTargetPawn);

protected:
	// Attack 
	// Attack range
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackRange = 150.0f;
	// Attack interval
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackInterval = 2.0f;
	//UPROPERTY()
	//FTimerHandle AttackTimerHandle;
	// Can attack
	UPROPERTY(VisibleInstanceOnly, Category = "Combat")
	bool bCanAttack = true;
	UPROPERTY(VisibleInstanceOnly, Category = "Combat")
	bool bIsAttacking = false;
	// Attack animation
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* AttackMontage;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Function to handle hit event
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void OnHit();

	UFUNCTION()
	int32 GetAttackRange() const { return AttackRange; }
	UFUNCTION()
	int32 GetDetectRange() const { return DetectRange; }
	UFUNCTION()
	float GetAttackInterval() const { return AttackInterval; }

	// Function to check if can attack
	bool CanAttack() const { return bCanAttack; }

	// Perform attack
	void PerformAttack();

protected:
	// Function to handle death
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void Die();

// Net About
protected:
	// RepNotify for changes made to Health
	UFUNCTION()
	void OnRep_Health();

	// Health
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health)
	int32 Health = 100;

	// UFUNCTION()
	// 这里不能用UFUNCTION标记，因为FLifetimeProperty不是一个被反射的类型
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// AI Initialization
	void InitializeAIController();
};
