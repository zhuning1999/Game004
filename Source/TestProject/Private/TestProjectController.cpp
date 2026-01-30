// Fill out your copyright notice in the Description page of Project Settings.


#include "TestProjectController.h"
#include "WeaponBase.h"
#include "TestProject/TestProjectCharacter.h"
#include "Blueprint/UserWidget.h"
#include <ostream>

void ATestProjectController::OnWeaponAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	CachedCurrentAmmo = CurrentAmmo;
	CachedMaxAmmo = MaxAmmo;

	OnAmmoUIChanged.Broadcast(CachedCurrentAmmo, CachedMaxAmmo);

	GEngine->AddOnScreenDebugMessage(
		-1,
		3.f,
		FColor::Cyan,
		FString::Printf(
			TEXT("[PC] Ammo Sync: %d / %d"),
			CachedCurrentAmmo,
			CachedMaxAmmo
		)
	);
}

void ATestProjectController::OnWeaponChanged(AWeaponBase* NewWeapon)
{
	// UE_LOG(LogTemp, Warning, TEXT("OnWeaponChanged: NewWeapon=%s"), *GetNameSafe(NewWeapon));
	if (CachedWeapon)
	{
		CachedWeapon->OnAmmoChanged.RemoveAll(this);
	}

	CachedWeapon = NewWeapon;
	
	if (CachedWeapon)
	{
		CachedWeapon->OnAmmoChanged.AddDynamic(this, &ATestProjectController::OnWeaponAmmoChanged);

		// UE_LOG(LogTemp, Warning, TEXT("Sync ammo manually"));
		OnWeaponAmmoChanged(CachedWeapon->GetCurrentAmmo(), CachedWeapon->GetMaxAmmo()); // 对齐子弹数据
	}
}

void ATestProjectController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ATestProjectCharacter* MyCharacter = Cast<ATestProjectCharacter>(InPawn);
	if (MyCharacter)
	{
		MyCharacter->OnCurrentWeaponChanged.RemoveAll(this);
		MyCharacter->OnCurrentWeaponChanged.AddDynamic(this, &ATestProjectController::OnWeaponChanged);

		// Initialize with the current weapon
		OnWeaponChanged(MyCharacter->GetCurrentWeapon());
	} 
}

void ATestProjectController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	ATestProjectCharacter* MyCharacter = Cast<ATestProjectCharacter>(GetPawn());
	if (!MyCharacter)
		return;

	MyCharacter->OnCurrentWeaponChanged.RemoveAll(this);
	MyCharacter->OnCurrentWeaponChanged.AddDynamic(this, &ATestProjectController::OnWeaponChanged);

	// 同步一次当前武器
	OnWeaponChanged(MyCharacter->GetCurrentWeapon());
}

void ATestProjectController::BeginPlay()
{
	PlayerCameraManager->ViewPitchMin = -30.f;
	PlayerCameraManager->ViewPitchMax = 35.f;

	Super::BeginPlay();
	if (IsLocalController())
	{
		if (CombatHUDClass)
		{
			CombatHUD = CreateWidget<UUserWidget>(this, CombatHUDClass);

			if (CombatHUD)
			{
				CombatHUD->AddToViewport();
			}
		}
	}

}
