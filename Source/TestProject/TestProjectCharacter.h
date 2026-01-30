// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FireSourceInterface.h"
#include "Logging/LogMacros.h"
#include "WeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "TestProjectCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCurrentWeaponChanged,
	AWeaponBase*, NewWeapon
);

UCLASS(config=Game)
class ATestProjectCharacter : public ACharacter, public IFireSourceInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Attack Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	/** Crouch Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/** Fire Input Action */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	// Weapon properties
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeaponBase> WeaponClass = AWeaponBase::StaticClass();

	UPROPERTY(Replicated)
	AWeaponBase* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TArray<AWeaponBase*> Weapons;
public:
	UPROPERTY(BlueprintAssignable)
	FOnCurrentWeaponChanged OnCurrentWeaponChanged; // 监听事件：切换武器
protected:
	// Health properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	int32 CurrentHealth = 100;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	int32 MaxHealth = 100;

	// Aiming properties
	UPROPERTY(ReplicatedUsing = OnRep_AimState)
	bool bIsAiming = false;
	UPROPERTY(EditAnywhere, Category = "Camera|Aim")
	USceneComponent* DefaultCameraSocket;
	UPROPERTY(VisibleAnywhere, Category = "Camera|Aim")
	USceneComponent* AimCameraSocket;
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
	float CameraInterpSpeed = 10.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
	FVector DefaultSocketOffset = FVector(0.f, 60.f, 70.f);

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
	FVector AimSocketOffset = FVector(50.f, 90.f, 80.f);
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
	float DefaultArmLength = 320.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
	float AimArmLength = 200.f;

public:
	ATestProjectCharacter();

protected:
	/* BeginPlay */
	virtual void BeginPlay() override;

	/* Tick */
	virtual void Tick(float DeltaTime) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for attack input */ 
	UFUNCTION(BlueprintNativeEvent)
	void Attack();
	void AttackInput(const FInputActionValue& Value);
	void OnFire();

	/** Called for crouch input */
	void Crouch(const FInputActionValue& Value);
	void UnCrouch(const FInputActionValue& Value);

	/** Called for equip weapon input */
	void EquipWeapon(AWeaponBase* NewWeapon);

	// Replication
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;
			
protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns CurrentWeapon **/
	FORCEINLINE class AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

	// IInputInterface implementation
	UFUNCTION(BlueprintCallable, Category = "FireSourceInterface")
	virtual FVector GetFireStartPoint() const;
	UFUNCTION(BlueprintCallable, Category = "FireSourceInterface")
	virtual FVector GetFireDirection() const;
	UFUNCTION(BlueprintCallable, Category = "FireSourceInterface")
	virtual AController* GetFireController() const;

	// Server functions
	UFUNCTION(Server, Reliable)
	void ServerFire(FVector CameraLocation, FVector CameraFor);
	UFUNCTION()
	void OnRep_AimState();

	// Aiming functions
	virtual void SetAiming(bool bAiming);
	void StartAiming();
	void StopAiming();
};