// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestProjectGameMode.h"
#include "TestProjectGameState.h"
#include "Enemy.h"
#include "TestProjectCharacter.h"
#include "TestProjectController.h"
#include "UObject/ConstructorHelpers.h"

ATestProjectGameMode::ATestProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PCClass(
		TEXT("/Game/Controller/BP_TestProjectController")
	);
	if (PCClass.Class)
	{
		PlayerControllerClass = PCClass.Class;
	}

	static ConstructorHelpers::FClassFinder<AEnemy> Elass(
		TEXT("/Game/ThirdPerson/BP_Enemy")
	);
	if (Elass.Class)
	{
		EnemyClass = Elass.Class;
	}

	GameStateClass = ATestProjectGameState::StaticClass();
}

void ATestProjectGameMode::BeginPlay()
{
	Super::BeginPlay();
}

