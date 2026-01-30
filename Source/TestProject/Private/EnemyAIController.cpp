// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyAIController.h"
#include "Navigation/PathFollowingComponent.h"

AEnemyAIController::AEnemyAIController()
{
	InitializePerceptionSystem();
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	EnemyState = EEnemyState::Idle;
	// TargetPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState == EEnemyState::Dead)
	{
		return;
	}

	UpdateCurrentState();
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	PossessedPawn = InPawn;
	EnemyPawn = Cast<AEnemy>(PossessedPawn);
	EnemyPawn->OnEnemyDied.AddDynamic(this, &AEnemyAIController::OnPossessedPawnDestroyed);
	EnemyPawn->OnEnemyAttackFinished.AddDynamic(this, &AEnemyAIController::OnPossessedPawnAttackFinished);
}

bool AEnemyAIController::IsInAttackRange(APawn* TargetActor) const
{
	if (PossessedPawn && TargetActor)
	{
		float Distance = FVector::Dist(PossessedPawn->GetActorLocation(), TargetActor->GetActorLocation());
		return Distance <= EnemyPawn->GetAttackRange();
	}

	return false;
}

bool AEnemyAIController::IsInDetectRange(APawn* TargetActor) const
{
	if (PossessedPawn && TargetActor)
	{
		float Distance = FVector::Dist(PossessedPawn->GetActorLocation(), TargetActor->GetActorLocation());
		return Distance <= EnemyPawn->GetDetectRange();
	}

	return false;
}

void AEnemyAIController::UpdateCurrentState()
{
	switch (EnemyState)
	{
	case EEnemyState::Idle:
		UpdateIdleState();
		break;
	case EEnemyState::Chasing:
		UpdateChaseState();
		break;
	case EEnemyState::Attacking:
		UpdateAttackState();
		break;
	case EEnemyState::Dead:
		break;
	default:
		break;
	}
}

void AEnemyAIController::UpdateChaseState()
{
	if (IsInAttackRange(TargetPawn))
	{
		EnterAttackState();
		return ;
	}
	
	if (!IsInDetectRange(TargetPawn))
	{
		EnterIdleState();
	}
}

void AEnemyAIController::EnterChaseState()
{
	if (EnemyState == EEnemyState::Chasing)
	{
		return;
	}

	const FVector EnemyLocation = PossessedPawn->GetActorLocation();
	const FVector PlayerLocation = TargetPawn->GetActorLocation();
	const float Distance = FVector::Dist(EnemyLocation, PlayerLocation);

	EnemyState = EEnemyState::Chasing;
	UE_LOG(LogTemp, Log, TEXT("Enemy → Chasing"));
	MoveToActor(TargetPawn);
}

void AEnemyAIController::UpdateAttackState()
{
	if (!IsInAttackRange(TargetPawn) && IsInDetectRange(TargetPawn))
	{
		EnterChaseState();
	}
}

void AEnemyAIController::EnterAttackState()
{
	if (EnemyState == EEnemyState::Attacking)
	{
		return;
	}
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < EnemyPawn->GetAttackInterval())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Enemy → Attacking"));
	EnemyState = EEnemyState::Attacking;
	LastAttackTime = CurrentTime;
	StopMovement();
	EnemyPawn->PerformAttack();
}

void AEnemyAIController::UpdateIdleState()
{
	if (IsInDetectRange(TargetPawn))
	{
		if (IsInAttackRange(TargetPawn) && EnemyPawn->CanAttack()) {
			EnterAttackState();
		} 
		else {
			EnterChaseState();
		}
	}
}

void AEnemyAIController::EnterIdleState()
{
	if (EnemyState == EEnemyState::Idle)
	{
		return;
	}

	EnemyState = EEnemyState::Idle;
	StopMovement();
}

void AEnemyAIController::OnPossessedPawnDestroyed()
{
	EnemyState = EEnemyState::Dead;
	UE_LOG(LogTemp, Log, TEXT("Enemy → Dead"));
	StopMovement();
	UnPossess();
}

void AEnemyAIController::OnPossessedPawnAttackFinished()
{
	UE_LOG(LogTemp, Log, TEXT("Enemy → Idle"));
	EnterIdleState();
}

void AEnemyAIController::InitializePerceptionSystem()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SetPerceptionComponent(*PerceptionComponent);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = 500.f;
	SightConfig->LoseSightRadius = 800.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
		this,
		&AEnemyAIController::OnTargetPerceptionUpdated
	);
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!HasAuthority())
	{
		return;
	}

	APawn* SensedPawn = Cast<APawn>(Actor);
	if (!SensedPawn)
	{
		return;
	}

	if (!SensedPawn->IsPlayerControlled())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed() && EnemyState != EEnemyState::Dead)
	{
		// 看见玩家
		UE_LOG(LogTemp, Log, TEXT("Enemy sees %s"), *Actor->GetName());

		AEnemy* Enemy = Cast<AEnemy>(GetPawn());
		if (Enemy)
		{
			TargetPawn = SensedPawn;
			Enemy->SetTargetPawn(SensedPawn);
			EnterChaseState();
		}
	}
	else
	{
		// 丢失目标
	}
}

void AEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();

	// 这里做清理工作
	if (EnemyPawn)
	{
		EnemyPawn->OnEnemyDied.RemoveDynamic(
			this,
			&AEnemyAIController::OnPossessedPawnDestroyed
		);
	}
}
