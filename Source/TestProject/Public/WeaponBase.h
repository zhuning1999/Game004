// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FireSourceInterface.h"
#include "WeaponBase.generated.h"

class IFireSourceInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, CurrentAmmo, int32, MaxAmmo);

UCLASS()
class TESTPROJECT_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
	// Initialize weapon properties
	UFUNCTION(BlueprintCallable)
	virtual void SetupWeaponProperties(float InDamage, float InFireRange, float InFireRate, float InReloadTime, int32 InAmmoCapacity, bool bInIsAutomatic);

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnAmmoChanged OnAmmoChanged;

protected:
	// Weapon outlook mesh
	// 枪本体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;
	// 枪口火焰
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* MuzzleFX;
	// 射击命中效果
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ImpactFX;

	// Fire properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRange = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRate = 0.5f;
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadWrite, Category = "Weapon")
	float LastFireTime = -999.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireCooldownTime = 0.3f;

	// Reload properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ReloadTime = 2.0f;
	UPROPERTY(Replicated)
	int32 MaxAmmo = 30;
	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	int32 CurrentAmmo = 30;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsAutomatic = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FTimerHandle ReloadTimeHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bReloading = false;

	// Fire Source
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FireSourceInterface")
	TScriptInterface<class IFireSourceInterface> FireSource;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// RepNotify for Net 
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Fire the weapon
	UFUNCTION(BlueprintCallable)
	virtual void Fire(const FVector& Start, const FVector& Direction);

protected:
	// About Fire
	UFUNCTION(BlueprintCallable)
	virtual void FireFeedback(bool bWasSuccessful, AActor* HitActor);

	UFUNCTION(BlueprintCallable)
	bool CanFire();

	UFUNCTION(BlueprintCallable)
	void Reload();

public:
	// About Equip Weapon
	UFUNCTION()
	void OnEquip(USkeletalMeshComponent* CharacterMesh);
	UFUNCTION()
	void OnUnEquip();
	UFUNCTION()
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
	UFUNCTION()
	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }
	// Fire source interface
	// UFUNCTION(BlueprintCallable, Category = "FireSourceInterface")
	// TScriptInterface<class IFireSourceInterface> GetFireSource() const { return FireSource; }
	UFUNCTION(BlueprintCallable, Category = "FireSourceInterface")
	void SetFireSource(const TScriptInterface<class IFireSourceInterface>& InFireSource) { FireSource = InFireSource; }

	UFUNCTION()
	void OnRep_Ammo();

	// Play fire effects
	UFUNCTION(Unreliable, NetMulticast)
	virtual void PlayFireFX();
	UFUNCTION(Unreliable, NetMulticast)
	virtual void PlayImpactFX(const FHitResult& Hit);
};
