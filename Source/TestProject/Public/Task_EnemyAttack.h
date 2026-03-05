// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Task_EnemyBase.h"
#include "Task_EnemyAttack.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API UTask_EnemyAttack : public UTask_EnemyBase
{
	GENERATED_BODY()
	
	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) override;
};
