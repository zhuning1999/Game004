// Fill out your copyright notice in the Description page of Project Settings.


#include "TestProjectAnimInstance.h"
#include "GameFramework/Character.h"

void UTestProjectAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwningPawn == nullptr)
	{
		OwningPawn = TryGetPawnOwner();
	}

	if (ACharacter* Character = Cast<ACharacter>(OwningPawn))
	{
		bIsCrouching = Character->bIsCrouched;
	}
}

void UTestProjectAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
}
