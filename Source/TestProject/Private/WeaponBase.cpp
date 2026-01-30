// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Enemy.h"
#include "Net/UnrealNetwork.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetRelativeScale3D(FVector(0.5f));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AWeaponBase::SetupWeaponProperties(float InDamage, float InFireRange, float InFireRate, float InReloadTime, int32 InAmmoCapacity, bool bInIsAutomatic)
{
	Damage = InDamage;
	FireRange = InFireRange;
	FireRate = InFireRate;
	ReloadTime = InReloadTime;
	MaxAmmo = InAmmoCapacity;
	CurrentAmmo = MaxAmmo;
	bIsAutomatic = bInIsAutomatic;
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, CurrentAmmo);
	DOREPLIFETIME(AWeaponBase, MaxAmmo);
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBase::Fire(const FVector& Start, const FVector& Direction)
{
	if (CanFire())
	{
		PlayFireFX();
		CurrentAmmo--;
		OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);

		FVector FireStartPoint = Start;
		FVector FireEndPoint = Start + Direction * FireRange;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(GetOwner());

		FHitResult Hit;
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, FireStartPoint, FireEndPoint, ECC_Visibility, Params);
		FireFeedback(bHit, Hit.GetActor());
		LastFireTime = GetWorld()->GetTimeSeconds(); // 写在这里为了确保每次成功开火后都更新最后开火时间，防止日后AI调用Fire时无限制开火

		if (bHit)
		{
			PlayImpactFX(Hit);
		}
	}
	else if (!bReloading && CurrentAmmo == 0)
	{
		bReloading = true;
		GetWorldTimerManager().SetTimer(ReloadTimeHandle, this, &AWeaponBase::Reload, ReloadTime, false);
	}
}


static FString GetClassSafeName(const UObject* Object)
{
	// Safely get the class name of an object, or "None" if the object is null
	return Object ? Object->GetClass()->GetName() : TEXT("None");
}

void AWeaponBase::FireFeedback(bool bWasSuccessful, AActor* HitActor)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Miss"));
		return;
	}


	if (HitActor->IsA<AEnemy>())
	{
		// UE_LOG(LogTemp, Log, TEXT("Hit Pawn: %s"), *GetNameSafe(HitActor));
		AEnemy* HitEnemy = Cast<AEnemy>(HitActor);
		HitEnemy->OnHit();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Hit Actor: %s"), *HitActor->GetName());
	}
	
	FVector FireStartPoint = FireSource->GetFireStartPoint();
	FVector FireDirection = FireSource->GetFireDirection();
	FVector FireEndPoint = FireStartPoint + FireDirection * FireRange;

	DrawDebugLine(
		GetWorld(),
		FireStartPoint,
		FireEndPoint,
		FColor::Red,
		false,
		1.0f,
		0,
		1.0f
	);

}

bool AWeaponBase::CanFire()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime >= FireCooldownTime && CurrentAmmo > 0 && !bReloading)
	{
		return true;
	}

	return false;
}

void AWeaponBase::Reload()
{
	// Sleep or wait for ReloadTime seconds in actual implementation
	CurrentAmmo = MaxAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
	bReloading = false;
}

void AWeaponBase::OnEquip(USkeletalMeshComponent* CharacterMesh)
{
	// Attach weapon to character mesh
	if (!CharacterMesh)
	{
		return;
	}
	// Attach to the "WeaponSocket" socket on the character mesh
	AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
	SetActorHiddenInGame(false);
}

void AWeaponBase::OnUnEquip()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorHiddenInGame(true);
}

void AWeaponBase::OnRep_Ammo()
{
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] OnRep_CurrentAmmo Fired: %d / %d"),
		CurrentAmmo, MaxAmmo);
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
}

void AWeaponBase::PlayFireFX_Implementation()
{
	if (!MuzzleFX || !WeaponMesh) return;

	UGameplayStatics::SpawnEmitterAttached(
		MuzzleFX,
		WeaponMesh,
		FName("MuzzleSocket"),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget
	);
}

void AWeaponBase::PlayImpactFX_Implementation(const FHitResult& Hit)
{
	if (!ImpactFX) return;

	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		ImpactFX,
		Hit.ImpactPoint,
		Hit.ImpactNormal.Rotation()
	);
}

