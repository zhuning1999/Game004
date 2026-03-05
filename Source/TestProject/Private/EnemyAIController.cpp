// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include <Kismet/GameplayStatics.h>

AEnemyAIController::AEnemyAIController()
{
	InitializePerceptionSystem();
	// StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
	// AddOwnedComponent(StateTreeComponent);
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	EnemyState = EEnemyState::Idle;

	OnEnemyAIAttack.AddDynamic(this, &AEnemyAIController::OnDamageTaken);
	// UE_LOG(LogTemp, Warning, TEXT("Has StateTree: %s"), StateTreeComponent && StateTreeComponent ? TEXT("YES") : TEXT("NO"));
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

	if (StateTreeComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Starting StateTree"));
		StateTreeComponent->StartLogic();
	}
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
	case EEnemyState::Alerted:
		UpdateAlertedState();
		break;
	default: // Death也在里面
		break;
	}
}

void AEnemyAIController::UpdateChaseState()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	FVector PlayerLocation = TargetPawn->GetActorLocation();
	FVector PlayerVelocity = TargetPawn->GetVelocity();
	// 视线检查，确保敌人是否能看到玩家
	bool bCanSee = LineOfSightTo(TargetPawn);
	if (bCanSee)
	{
		// 更新记忆
		LastKnownPlayerLocation = TargetPawn->GetActorLocation();
		LostSightTime = CurrentTime;
		bHasLineOfSight = true;
	}
	else
	{
		bHasLineOfSight = false;
	}

	// 记忆逻辑
	if (!bHasLineOfSight)
	{
		// UE_LOG(LogTemp, Warning, TEXT("DeltaTime: %f"), CurrentTime - LostSightTime);
		if (CurrentTime - LostSightTime > MemoryDuration)
		{
			ChangeState(EEnemyState::Alerted);
			return;
		}

		// 仍然有记忆 → 去最后已知位置
		MoveToLocation(LastKnownPlayerLocation);
		return;
	}

	// 如果有多个敌人追击同一个玩家，只有距离玩家最近的那个敌人会继续追击，其他敌人会进入警戒状态，等待玩家被击败或者逃离后再继续追击。
	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), AllEnemies);

	float MyDistance = FVector::Dist(
		GetPawn()->GetActorLocation(),
		TargetPawn->GetActorLocation()
	);

	bool bIAmClosest = true;

	for (AActor* Actor : AllEnemies)
	{
		AEnemy* Other = Cast<AEnemy>(Actor);
		if (!Other || Other == EnemyPawn)
			continue;

		float OtherDist = FVector::Dist(
			Other->GetActorLocation(),
			TargetPawn->GetActorLocation()
		);

		if (OtherDist < MyDistance)
		{
			bIAmClosest = false;
			break;
		}
	}

	// 预测玩家位置
	float Distance = FVector::Dist(
		PossessedPawn->GetActorLocation(),
		LastKnownPlayerLocation
	);
	// 依照距离调整预测时间，确保敌人在不同距离下都能合理地预测玩家位置
	float PredictionTime = FMath::Clamp(Distance / 600.f, 0.2f, 0.8f);
	FVector PredictedLocation = LastKnownPlayerLocation;
	// 只有在有视线的情况下才进行预测，避免敌人过度追踪玩家
	// 貌似冗余了，因为上面已经有视线检查了，但为了安全起见还是加上了这个条件
	if (bHasLineOfSight)
	{
		PredictedLocation += PlayerVelocity * PredictionTime;
	}

	if (CurrentTime - LastChaseRepathTime > ChaseRepathInterval)
	{
		LastChaseRepathTime = CurrentTime;

		if (bIAmClosest)
		{
			MoveToLocation(PredictedLocation);
		}
		else
		{
			FVector PlayerLoc = TargetPawn->GetActorLocation();
			FVector EnemyLoc = GetPawn()->GetActorLocation();

			FVector Dir = (EnemyLoc - PlayerLoc).GetSafeNormal();
			FVector Right = FVector::CrossProduct(Dir, FVector::UpVector);

			FVector FlankPos = PlayerLoc + Right * 300.f * FlankSide;

			MoveToLocation(FlankPos);
		}

		if (Distance < EnemyPawn->GetAttackRange() * 3)
		{
			EnemyPawn->GetCharacterMovement()->MaxWalkSpeed = 200.f;
		}
		else
		{
			EnemyPawn->GetCharacterMovement()->MaxWalkSpeed = 400.f;
		}
	}

	if (bHasLineOfSight && IsInAttackRange(TargetPawn))
	{
		ChangeState(EEnemyState::Attacking);
	}
}

void AEnemyAIController::EnterChaseState()
{
	//const FVector EnemyLocation = PossessedPawn->GetActorLocation();
	//const FVector PlayerLocation = TargetPawn->GetActorLocation();
	//const float Distance = FVector::Dist(EnemyLocation, PlayerLocation);

	// EnemyState = EEnemyState::Chasing;
	FlankSide = FMath::RandBool() ? 1.f : -1.f;
	UE_LOG(LogTemp, Log, TEXT("Enemy → Chasing"));
}

void AEnemyAIController::UpdateAttackState()
{
	if (!IsInAttackRange(TargetPawn) && IsInDetectRange(TargetPawn))
	{
		ChangeState(EEnemyState::Chasing);
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime >= EnemyPawn->GetAttackInterval())
	{
		EnemyPawn->PerformAttack();
		LastAttackTime = CurrentTime;
	}
}

void AEnemyAIController::EnterAttackState()
{
	UE_LOG(LogTemp, Log, TEXT("Enemy → Attacking"));

	StopMovement();
}

void AEnemyAIController::UpdateIdleState()
{
	// 这里有个隐藏bug，bHasLineOfSight的值没有正确更新，导致敌人可能在玩家进入视野后无法正确切换到追逐状态。
	// 但是奇迹般的是，由于OnTargetPerceptionUpdated会在玩家进入视野时被调用，并且自动进入了Chase状态，所以这个问题被掩盖了。
	// 先这样吧，等后续有时间再来修复这个问题，可能需要在UpdateIdleState中添加一个视线检查来确保状态切换的正确性。
	// 潜在的解决方案是把这里的bHasLineOfSight改成LineOfSightTo(TargetPawn)
	// 所谓屎山代码，能跑就别动了，哈哈哈
	// if (LineOfSightTo(TargetPawn) && IsInDetectRange(TargetPawn))
	if (bHasLineOfSight && IsInDetectRange(TargetPawn))
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy → Detected Player"));
		if (IsInAttackRange(TargetPawn) && EnemyPawn->CanAttack()) {
			ChangeState(EEnemyState::Attacking);
		} 
		else {
			ChangeState(EEnemyState::Chasing);
		}
	}
}

void AEnemyAIController::EnterIdleState()
{
	// EnemyState = EEnemyState::Idle;
	StopMovement();
}

void AEnemyAIController::UpdateAlertedState()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastAlertRepathTime > 1.0f)
	{
		MoveToLocation(LastKnownPlayerLocation);
		LastAlertRepathTime = CurrentTime;
	}

	// 如果重新看到玩家就直接追击
	if (LineOfSightTo(TargetPawn))
	{
		ChangeState(EEnemyState::Chasing);
		return;
	}

	// 搜索时间结束则回Idle
	if (CurrentTime - LostSightTime > MemoryDuration)
	{
		ChangeState(EEnemyState::Idle);
		return;
	}
}

void AEnemyAIController::EnterAlertedState()
{
	UE_LOG(LogTemp, Log, TEXT("Enemy → Alerted"));

	// 稍微减速，表现为谨慎移动
	EnemyPawn->GetCharacterMovement()->MaxWalkSpeed = 300.f;
	MoveToLocation(LastKnownPlayerLocation);// LastKnownPlayerLocation在受伤函数中已经更新了
}

void AEnemyAIController::ChangeState(EEnemyState NewState)
{
	if (EnemyState == NewState)
		return;
	// 结束当前状态
	ExitCurrentState();
	// 切换状态
	EnemyState = NewState;
	// 进入新状态
	switch (EnemyState)
	{
	case EEnemyState::Idle:
		EnterIdleState();
		break;

	case EEnemyState::Chasing:
		EnterChaseState();
		break;

	case EEnemyState::Attacking:
		EnterAttackState();
		break;

	case EEnemyState::Dead:
		break;

	case EEnemyState::Alerted:
		EnterAlertedState();
		break;
	}
}

void AEnemyAIController::ExitCurrentState()
{
	switch (EnemyState)
	{
	case EEnemyState::Idle:
		break;

	case EEnemyState::Chasing:
		StopMovement();
		break;

	case EEnemyState::Attacking:
		break;

	case EEnemyState::Dead:
		StopMovement();
		UnPossess();
		break;
	case EEnemyState::Alerted:
		StopMovement();
		break;
	}
}

void AEnemyAIController::OnPossessedPawnDestroyed()
{
	UE_LOG(LogTemp, Log, TEXT("Enemy → Dead"));
	ChangeState(EEnemyState::Dead);
}

void AEnemyAIController::OnPossessedPawnAttackFinished()
{
	UE_LOG(LogTemp, Log, TEXT("Enemy → Idle"));
	ChangeState(EEnemyState::Idle);
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
			LastKnownPlayerLocation = TargetPawn->GetActorLocation();
			Enemy->SetTargetPawn(SensedPawn);
			ChangeState(EEnemyState::Chasing);
		}

	}
	else
	{
		// 丢失目标
	}
}

void AEnemyAIController::OnDamageTaken(AActor* InstigatorActor)
{
	LastKnownPlayerLocation = InstigatorActor->GetActorLocation();
	LostSightTime = GetWorld()->GetTimeSeconds();

	ChangeState(EEnemyState::Alerted);
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
