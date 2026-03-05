// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Task_EnemyBase.h"
#include "Task_EnemyChase.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API UTask_EnemyChase : public UTask_EnemyBase
{
	GENERATED_BODY()
	
	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime) override;
};
