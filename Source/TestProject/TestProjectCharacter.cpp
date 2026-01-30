// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestProjectCharacter.h"
#include "TestProjectController.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATestProjectCharacter

ATestProjectCharacter::ATestProjectCharacter()
{
	// Replicate to others or not
	bReplicates = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = AActor::CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = AActor::CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	DefaultCameraSocket = CreateDefaultSubobject<USceneComponent>(TEXT("CamSocket_Third"));
	DefaultCameraSocket->SetupAttachment(RootComponent);
	DefaultCameraSocket->SetRelativeLocation(FVector(-300.f, 0.f, 80.f));

	AimCameraSocket = CreateDefaultSubobject<USceneComponent>(TEXT("CamSocket_Aim"));
	AimCameraSocket->SetupAttachment(RootComponent);
	AimCameraSocket->SetRelativeLocation(FVector(-120.f, 40.f, 70.f));



	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATestProjectCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Bind weapon
	if (WeaponClass)
	{
		if (HasAuthority())
		{
			AWeaponBase* PrimaryWeapon = AActor::GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);
			if (PrimaryWeapon)
			{
				PrimaryWeapon->SetupWeaponProperties(15.0f, 1500.0f, 0.2f, 1.5f, 25, true);
				Weapons.Add(PrimaryWeapon);
			}

			AWeaponBase* SecondaryWeapon = AActor::GetWorld()->SpawnActor<AWeaponBase>(WeaponClass);
			if (SecondaryWeapon)
			{
				SecondaryWeapon->SetupWeaponProperties(15.0f, 1500.0f, 0.2f, 1.5f, 30, true);
				Weapons.Add(SecondaryWeapon);
				SecondaryWeapon->OnUnEquip();
			}
		}
	}

	if (Weapons.Num() > 0)
	{
		EquipWeapon(Weapons[0]);
	}
}

void ATestProjectCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector TargetOffset = bIsAiming ? AimSocketOffset : DefaultSocketOffset;
	const float TargetArm = bIsAiming ? AimArmLength : DefaultArmLength;

	CameraBoom->SocketOffset = FMath::VInterpTo(
		CameraBoom->SocketOffset,
		TargetOffset,
		DeltaTime,
		CameraInterpSpeed
	);

	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		TargetArm,
		DeltaTime,
		CameraInterpSpeed
	);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATestProjectCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATestProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATestProjectCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATestProjectCharacter::Look);

		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ATestProjectCharacter::AttackInput);

		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ATestProjectCharacter::OnFire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATestProjectCharacter::SetAiming, false);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATestProjectCharacter::StartAiming);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATestProjectCharacter::StopAiming);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ATestProjectCharacter::Crouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ATestProjectCharacter::UnCrouch);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

FVector ATestProjectCharacter::GetFireStartPoint() const
{
	int32 X, Y;
	auto PC = Cast<APlayerController>(GetController());
	PC->GetViewportSize(X, Y);

	FVector Start, Dir;
	PC->DeprojectScreenPositionToWorld(
		X * 0.5f,
		Y * 0.5f,
		Start,
		Dir
	);

	return Start;
}

FVector ATestProjectCharacter::GetFireDirection() const
{
	int32 X, Y;
	auto PC = Cast<APlayerController>(GetController());
	PC->GetViewportSize(X, Y);

	FVector Start, Dir;
	PC->DeprojectScreenPositionToWorld(
		X * 0.5f,
		Y * 0.5f,
		Start,
		Dir
	);

	return Dir;
}

AController* ATestProjectCharacter::GetFireController() const
{
	return GetController();
}

void ATestProjectCharacter::OnRep_AimState()
{
	if (IsLocallyControlled())
	{
		if (auto* PC = Cast<ATestProjectController>(GetController())) {
			PC->SetAimingUI(bIsAiming);
		}
	}

	bUseControllerRotationPitch = bIsAiming;
	GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
}

void ATestProjectCharacter::SetAiming(bool bAiming)
{
	if (bIsAiming == bAiming)
	{
		return;
	}

	bIsAiming = bAiming;
	OnRep_AimState();
}

void ATestProjectCharacter::StartAiming()
{
	if (bIsAiming) return;

	bIsAiming = true;

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	CameraBoom->bUsePawnControlRotation = false;
	FollowCamera->bUsePawnControlRotation = true;
}

void ATestProjectCharacter::StopAiming()
{
	if (!bIsAiming) return;

	bIsAiming = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	CameraBoom->bUsePawnControlRotation = true;
	FollowCamera->bUsePawnControlRotation = false;
}

void ATestProjectCharacter::ServerFire_Implementation(FVector CameraLocation, FVector CameraForward)
{
	if (CurrentWeapon) {
		CurrentWeapon->Fire(CameraLocation, CameraForward);
	}
}


void ATestProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATestProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATestProjectCharacter::AttackInput(const FInputActionValue& Value)
{
	Attack();
}

void ATestProjectCharacter::OnFire()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	FVector CameraLocation = FollowCamera->GetComponentLocation();
	FRotator CameraRotation = FollowCamera->GetComponentRotation();
	FVector CameraForward = CameraRotation.Vector();
	ServerFire(CameraLocation, CameraForward); // 客户端请求服务器执行
	SetAiming(true);
}

void ATestProjectCharacter::Crouch(const FInputActionValue& Value)
{
	Super::Crouch();
	// 个人建议，最好把胶囊体的高度改一下
	// 不过貌似UE默认的Crouch已经改了胶囊体高度
	// UE_LOG(LogTemplateCharacter, Log, TEXT("Crouch action triggered"));
}

void ATestProjectCharacter::UnCrouch(const FInputActionValue& Value)
{
	Super::UnCrouch();
	// UE_LOG(LogTemplateCharacter, Log, TEXT("UnCrouch action triggered"));
}

void ATestProjectCharacter::EquipWeapon(AWeaponBase* NewWeapon)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->OnUnEquip();
	}
	CurrentWeapon = NewWeapon;
	if (CurrentWeapon)
	{
		CurrentWeapon->OnEquip(this->GetMesh());
	}
	CurrentWeapon->SetFireSource(this);
	OnCurrentWeaponChanged.Broadcast(CurrentWeapon);
}

void ATestProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATestProjectCharacter, bIsAiming);
	DOREPLIFETIME(ATestProjectCharacter, CurrentWeapon);
}

void ATestProjectCharacter::Attack_Implementation()
{
	// Implement attack logic here
	UE_LOG(LogTemplateCharacter, Log, TEXT("Attack action triggered"));
}
