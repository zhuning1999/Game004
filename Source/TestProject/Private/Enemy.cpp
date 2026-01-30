// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h"
#include <TestProject/TestProjectCharacter.h>

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AEnemyAIController::StaticClass(); // Defined in APawn.h
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		InitializeAIController();
	}

	TargetPawn = GetWorld()->GetFirstPlayerController()->GetPawn(); // 单机游戏下暂时这样，同时也为了防止null指针
}

void AEnemy::SetTargetPawn(APawn* NewTargetPawn)
{
	if (!HasAuthority())
	{
		return;
	}

	TargetPawn = NewTargetPawn;

	UE_LOG(LogTemp, Log, TEXT("Player target set to %s"), *NewTargetPawn->GetName());
}


void AEnemy::OnHit()
{
	if (!HasAuthority())
	{
		return;
	}

	Health -= Health > 0 ? 50 : 0;
	
	if (Health <= 0)
	{
		Die();
	}
}

void AEnemy::Die()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	OnEnemyDied.Broadcast();
	// 延迟销毁（给动画时间）
	SetLifeSpan(3.f);
}

void AEnemy::OnRep_Health()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Red,
			FString::Printf(TEXT("Health updated: %d"), Health) // 这里之前误用了%f结果导致显示为0 :<
		);
	}
}

void AEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEnemy, Health);
}

void AEnemy::InitializeAIController()
{
	AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController());
	if (!AICon)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy has no AIController on server"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Enemy has AIController on server"));
}

void AEnemy::PerformAttack()
{
	if (AEnemyAIController* EnemyAIController = Cast<AEnemyAIController>(GetController()))
	{
		APawn* PlayerPawn = TargetPawn;
		if (PlayerPawn)
		{
			if (ATestProjectCharacter* PlayerCharacter = Cast<ATestProjectCharacter>(PlayerPawn))
			{
				if (!AttackMontage)
				{
					UE_LOG(LogTemp, Warning, TEXT("AttackMontage is null"));
					return;
				}

				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if (!AnimInstance)
				{
					UE_LOG(LogTemp, Warning, TEXT("AnimInstance is null"));
					return;
				}
				AnimInstance->Montage_Play(AttackMontage);
			}
		}
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

