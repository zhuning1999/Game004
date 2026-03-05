// Fill out your copyright notice in the Description page of Project Settings.


#include "Task_EnemyBase.h"

const FEnemyStateTreeData* UTask_EnemyBase::GetEnemyStateTreeContext(FStateTreeExecutionContext& Context) const
{
	// 先这么写吧，反正现在只有一个Struct，后续如果有多个Struct再改成枚举区分
	return Context.GetInstanceData() ? Context.GetInstanceData()->GetStruct(0).GetPtr<FEnemyStateTreeData>() : nullptr;
}
