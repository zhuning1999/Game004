// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyStateTreeData.generated.h"

/**
 * 
 */
// AI状态枚举
UENUM(BlueprintType)
enum class EEnemyTacticalState : uint8
{
    Idle,
    Chasing,
    Attacking,
    Searching,
    Dead
};

USTRUCT(BlueprintType)
struct FEnemyStateTreeData
{
    GENERATED_BODY()
	// 目标Pawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    TWeakObjectPtr<APawn> TargetPawn;
	// 最后已知位置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    FVector LastKnownLocation;
	// 怀疑程度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float SuspicionLevel; // 0.0 - 100.0
	// 最后一次攻击时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    float LastAttackTime;
	// 是否死亡
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    bool bIsDead = false;

    FEnemyStateTreeData()
        : TargetPawn(nullptr)
        , LastKnownLocation(FVector::ZeroVector)
        , SuspicionLevel(0.0f)
        , LastAttackTime(0.0f)
        , bIsDead(false)
    {
    }
};