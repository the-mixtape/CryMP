// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CMPCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Anim/CMPAnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Guns/GunParent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/CMPCharacterMovementComponent.h"


ACMPCharacter::ACMPCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCMPCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GetCharacterMovement()->SetIsReplicated(true);
	CMPCharacterMovementComponent = Cast<UCMPCharacterMovementComponent>(GetCharacterMovement());

	FPCamera = CreateDefaultSubobject<UCameraComponent>("FPCamera");
	FPCamera->SetupAttachment(GetMesh(), "CameraSocket");
	FPCamera->bUsePawnControlRotation = true;
}

void ACMPCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (HasAuthority())
	{
		SpawnGunsInventory();
	}
}

void ACMPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACMPCharacter, CurrentWeapon);
	DOREPLIFETIME(ACMPCharacter, HandTransform);
	DOREPLIFETIME(ACMPCharacter, bInterpolateSight);
}

void ACMPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACMPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACMPCharacter::JumpPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
		                                   &ACMPCharacter::JumpReleased);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACMPCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACMPCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACMPCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &ACMPCharacter::Look);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ACMPCharacter::AimStarted);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &ACMPCharacter::AimFinished);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ACMPCharacter::AimFinished);

		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ACMPCharacter::RunStarted);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Canceled, this, &ACMPCharacter::RunFinished);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ACMPCharacter::RunFinished);

		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ACMPCharacter::WalkStarted);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Canceled, this, &ACMPCharacter::WalkFinished);
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &ACMPCharacter::WalkFinished);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this,
		                                   &ACMPCharacter::CrouchPressed);
	}
}

void ACMPCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (!GetAnimInstance()) return;
	if (PrevMovementMode != MOVE_Walking || GetCharacterMovement()->MovementMode != MOVE_Falling || !bIsAiming) return;

	ExitAiming();
}

void ACMPCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	InputAxisRight = MovementVector.X;

	AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	AddMovementInput(GetActorRightVector(), InputAxisRight);
}

void ACMPCharacter::Look(const FInputActionValue& Value)
{
	if (!Controller) return;

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	InputAxisYaw = LookAxisVector.X;
	InputAxisPitch = LookAxisVector.Y;

	AddControllerYawInput(InputAxisYaw);
	AddControllerPitchInput(InputAxisPitch);
}

void ACMPCharacter::AimStarted(const FInputActionValue& Value)
{
	if (!CurrentWeapon) return;

	if (UKismetMathLibrary::EqualEqual_TransformTransform(HandTransform, FTransform()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow,
		                                 TEXT("Can't aim down sights, gun has no sight position values."));
		return;
	}

	if (!bIsAiming)
	{
		EnterAiming();
	}
	else if (!bIsHoldToAim)
	{
		ExitAiming();
	}
}

void ACMPCharacter::AimFinished(const FInputActionValue& Value)
{
	if (!CurrentWeapon) return;

	if (bIsHoldToAim && bIsAiming)
	{
		ExitAiming();
	}
}

void ACMPCharacter::RunStarted(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (CMPCharacterMovementComponent->IsCrouching())
	{
		CMPCharacterMovementComponent->StopCrouch();
		return;
	}

	CMPCharacterMovementComponent->StartRun();
}

void ACMPCharacter::RunFinished(const FInputActionValue& Value)
{
	if (!Controller) return;

	CMPCharacterMovementComponent->StopRun();
}

void ACMPCharacter::WalkStarted(const FInputActionValue& Value)
{
	if (!Controller) return;

	bIsWalkPressed = true;
	CMPCharacterMovementComponent->StartWalk();
}

void ACMPCharacter::WalkFinished(const FInputActionValue& Value)
{
	if (!Controller) return;

	bIsWalkPressed = false;
	CMPCharacterMovementComponent->StopWalk();
}

void ACMPCharacter::CrouchPressed(const FInputActionValue& Value)
{
	if (!Controller) return;

	CMPCharacterMovementComponent->ToggleCrouch();
}

void ACMPCharacter::JumpPressed(const FInputActionValue& Value)
{
	if (CMPCharacterMovementComponent->IsCrouching())
	{
		CMPCharacterMovementComponent->StopCrouch();
		return;
	}

	ACharacter::Jump();
}

void ACMPCharacter::JumpReleased(const FInputActionValue& Value)
{
	ACharacter::StopJumping();
}

void ACMPCharacter::SetIsAiming(bool InIsAiming)
{
	bIsAiming = InIsAiming;
	CurrentWeapon->bIsAiming = bIsAiming;

	const auto AnimInstance = GetAnimInstance();
	if(!AnimInstance) return;
	AnimInstance->bIsAiming = bIsAiming;

	if(bIsAiming)
	{
		CMPCharacterMovementComponent->StartWalk();
	}
	else if(!bIsWalkPressed)
	{
		CMPCharacterMovementComponent->StopWalk();
	}
}

void ACMPCharacter::OnRep_CurrentWeapon()
{
	const auto AnimInstance = GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->SwitchWeapon();
}

void ACMPCharacter::OnRep_HandTransform()
{
	if (UKismetMathLibrary::EqualEqual_TransformTransform(HandTransform, FTransform()))
	{
		ExitAiming();
		return;
	}

	const auto AnimInstance = GetAnimInstance();
	if (!AnimInstance) return;

	AnimInstance->NewHandTransform(bInterpolateSight, HandTransform);
}

void ACMPCharacter::EnterAiming()
{
	const auto AnimInstance = GetAnimInstance();
	if (!AnimInstance) return;

	if (CMPCharacterMovementComponent->IsFalling() ||
		CMPCharacterMovementComponent->GetCustomMovementState() == ECustomMovementState::ECMS_Run ||
		AnimInstance->IsPlayingSlotAnimation(AnimInstance->UnEquipSequence, AnimInstance->EquipAnimSlotName) ||
		AnimInstance->IsPlayingSlotAnimation(AnimInstance->EquipSequence, AnimInstance->EquipAnimSlotName))
		return;

	ServerEnterAiming();
	SetIsAiming(true);
}

void ACMPCharacter::ExitAiming()
{
	ServerExitAiming();
	SetIsAiming(false);
}

void ACMPCharacter::ServerEnterAiming_Implementation()
{
	MulticastEnterAiming();
}

void ACMPCharacter::MulticastEnterAiming_Implementation()
{
	if (IsLocallyControlled()) return;

	SetIsAiming(true);
}

void ACMPCharacter::ServerExitAiming_Implementation()
{
	MulticastExitAiming();
}

void ACMPCharacter::MulticastExitAiming_Implementation()
{
	if(IsLocallyControlled()) return;
	
	SetIsAiming(false);
}

void ACMPCharacter::SwitchWeapon()
{
	if (LastWeapon)
	{
		UnEquipLastWeapon();
	}

	EquipCurrentWeapon();
}

void ACMPCharacter::SpawnGunsInventory()
{
	TArray<TSubclassOf<AGunParent>> GunClasses;
	StartingGunsInventory.GetKeys(GunClasses);

	for (const auto GunClass : GunClasses)
	{
		const auto Quantity = *StartingGunsInventory.Find(GunClass);
		for (uint8 Index = 0; Index < Quantity; Index++)
		{
			FActorSpawnParameters Params;
			Params.Owner = this;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Params.TransformScaleMethod = ESpawnActorScaleMethod::SelectDefaultAtRuntime;
			
			if (const auto Gun = GetWorld()->SpawnActor<AGunParent>(
				GunClass.Get(), FVector::ZeroVector, FRotator::ZeroRotator, Params))
			{
				AddToGunInventory(Gun);
			}
		}
	}

	if (GunsInventory.Num() > 0)
	{
		if (const auto FirstGun = *GunsInventory.GetData())
		{
			CurrentWeapon = FirstGun;
			if (GetNetMode() == NM_ListenServer)
			{
				OnRep_CurrentWeapon();
			}
		}
	}
}

void ACMPCharacter::AddToGunInventory(AGunParent* Gun)
{
	const FAttachmentTransformRules AttachRules(
		EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
	Gun->AttachToComponent(GetMesh(), AttachRules, GunHolsterSocketName);
	GunsInventory.AddUnique(Gun);
}

void ACMPCharacter::UnEquipLastWeapon()
{
	if (HasAuthority())
	{
		const FAttachmentTransformRules AttachRules(
			EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
		LastWeapon->AttachToComponent(GetMesh(), AttachRules, GunHolsterSocketName);
	}

	if (IsLocallyControlled() && bIsAiming)
	{
		ExitAiming();
	}

	LastWeapon->UnEquip();
}

void ACMPCharacter::EquipCurrentWeapon()
{
	if (HasAuthority())
	{
		const FAttachmentTransformRules AttachRules(
			EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
		CurrentWeapon->AttachToComponent(GetMesh(), AttachRules, GunSocketName);
		CurrentWeapon->CalculateHandTransforms();
	}

	UpdateWeaponAnims();

	CurrentWeapon->Equip();
	LastWeapon = CurrentWeapon;
}

void ACMPCharacter::UpdateWeaponAnims()
{
	const auto AnimInstance = GetAnimInstance();
	if (!AnimInstance) return;

	GetMesh()->LinkAnimClassLayers(CurrentWeapon->AnimLayer);
	AnimInstance->EquipSequence = CurrentWeapon->EquipSequence;
	AnimInstance->UnEquipSequence = CurrentWeapon->UnEquipSequence;
	AnimInstance->FireMontage = CurrentWeapon->FireMontage;
}

UCMPAnimInstance* ACMPCharacter::GetAnimInstance()
{
	if (!CMPAnimInstance && GetMesh())
	{
		if (const auto CurrentAnimInstance = GetMesh()->GetAnimInstance())
		{
			CMPAnimInstance = Cast<UCMPAnimInstance>(CurrentAnimInstance);
		}
	}

	return CMPAnimInstance;
}
