// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WeaponBase.h"
#include "TestProjectController.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAmmoUIChanged,
	int32, CurrentAmmo,
	int32, MaxAmmo
);

UCLASS()
class TESTPROJECT_API ATestProjectController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnWeaponAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo); // 更新子弹数据
	UFUNCTION(Category = "Weapon")
	void OnWeaponChanged(AWeaponBase* NewWeapon); // 切换武器时调用

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void OnPossess(APawn* InPawn) override;

	
	virtual void OnRep_Pawn() override;

	UFUNCTION(BlueprintPure)
	int32 GetCurrentAmmo() const { return CachedCurrentAmmo; }
	UFUNCTION(BlueprintPure)
	int32 GetMaxAmmo() const { return CachedMaxAmmo; }
	
protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	int32 CachedCurrentAmmo = -1;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	int32 CachedMaxAmmo = 0;
	AWeaponBase* CachedWeapon = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> CombatHUDClass;
	UPROPERTY()
	UUserWidget* CombatHUD;
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnAmmoUIChanged OnAmmoUIChanged;

	virtual void BeginPlay() override;

public:
	// Aiming UI
	UFUNCTION(BlueprintImplementableEvent, Category = "Aiming")
	void SetAimingUI(bool bAiming);
};
