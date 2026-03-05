// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "StateTreeNodeBase.h"
#include "ST_Condition_HasTarget.generated.h"


USTRUCT(meta = (DisplayName = "Has Target"))
struct TESTPROJECT_API FST_Condition_HasTarget : public FStateTreeConditionBase
{
    GENERATED_BODY()

    virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
