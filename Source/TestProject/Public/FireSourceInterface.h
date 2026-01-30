// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FireSourceInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UFireSourceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TESTPROJECT_API IFireSourceInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

    // 射线起点
	UFUNCTION()
    virtual FVector GetFireStartPoint() const = 0;

    // 射线方向
    UFUNCTION()
    virtual FVector GetFireDirection() const = 0;

    // 用于伤害系统 / 权限归属
    UFUNCTION()
    virtual AController* GetFireController() const = 0;
};
