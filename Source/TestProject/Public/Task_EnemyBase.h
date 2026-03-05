// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeInstanceData.h"
#include "EnemyStateTreeData.h"
#include "Enemy.h"
#include "Task_EnemyBase.generated.h"

/**
 * 
 */
UCLASS()
class TESTPROJECT_API UTask_EnemyBase : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:
	const FEnemyStateTreeData* GetEnemyStateTreeContext(FStateTreeExecutionContext& Context) const;
	AEnemy* GetEnemy(FStateTreeExecutionContext& Context) const
	{
		return Cast<AEnemy>(Context.GetOwner());
	}
};