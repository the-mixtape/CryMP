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


FSharedRepMovement::FSharedRepMovement()
{
	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
}

bool FSharedRepMovement::FillForCharacter(ACharacter* Character)
{
	if (USceneComponent* PawnRootComponent = Character->GetRootComponent())
	{
		UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

		RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);
		RepMovement.Rotation = PawnRootComponent->GetComponentRotation();
		RepMovement.LinearVelocity = CharacterMovement->Velocity;
		RepMovementMode = CharacterMovement->PackNetworkMovementMode();
		bProxyIsJumpForceApplied = Character->bProxyIsJumpForceApplied || (Character->JumpForceTimeRemaining > 0.0f);
		bIsCrouched = Character->bIsCrouched;

		// Timestamp is sent as zero if unused
		if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
		{
			RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
		}
		else
		{
			RepTimeStamp = 0.f;
		}

		return true;
	}
	return false;
}

bool FSharedRepMovement::Equals(const FSharedRepMovement& Other, ACharacter* Character) const
{
	if (RepMovement.Location != Other.RepMovement.Location)
	{
		return false;
	}

	if (RepMovement.Rotation != Other.RepMovement.Rotation)
	{
		return false;
	}

	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity)
	{
		return false;
	}

	if (RepMovementMode != Other.RepMovementMode)
	{
		return false;
	}

	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied)
	{
		return false;
	}

	if (bIsCrouched != Other.bIsCrouched)
	{
		return false;
	}

	return true;
}

bool FSharedRepMovement::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	RepMovement.NetSerialize(Ar, Map, bOutSuccess);
	Ar << RepMovementMode;
	Ar << bProxyIsJumpForceApplied;
	Ar << bIsCrouched;

	// Timestamp, if non-zero.
	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
	Ar.SerializeBits(&bHasTimeStamp, 1);
	if (bHasTimeStamp)
	{
		Ar << RepTimeStamp;
	}
	else
	{
		RepTimeStamp = 0.f;
	}

	return true;
}

ACMPCharacter::ACMPCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCMPCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	GetCharacterMovement()->SetIsReplicated(true);
	CMPCharacterMovementComponent = Cast<UCMPCharacterMovementComponent>(GetCharacterMovement());
	CMPCharacterMovementComponent->bAllowPhysicsRotationDuringAnimRootMotion = false;
	CMPCharacterMovementComponent->SetIsReplicated(true);

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
	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
}

void ACMPCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (CMPCharacterMovementComponent)
	{
		// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
		const double MaxAccel = CMPCharacterMovementComponent->MaxAcceleration;
		const FVector CurrentAccel = CMPCharacterMovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

		ReplicatedAcceleration.AccelXYRadians = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);
		// [0, 2PI] -> [0, 255]
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);
		// [0, MaxAccel] -> [0, 255]
		ReplicatedAcceleration.AccelZ = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);
		// [-MaxAccel, MaxAccel] -> [-127, 127]
	}
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

		EnhancedInputComponent->BindAction(JogAction, ETriggerEvent::Started, this, &ACMPCharacter::JogStarted);
		EnhancedInputComponent->BindAction(JogAction, ETriggerEvent::Canceled, this, &ACMPCharacter::JogFinished);
		EnhancedInputComponent->BindAction(JogAction, ETriggerEvent::Completed, this, &ACMPCharacter::JogFinished);

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

void ACMPCharacter::JogStarted(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (CMPCharacterMovementComponent->IsCrouching())
	{
		CMPCharacterMovementComponent->StopCrouch();
		return;
	}

	CMPCharacterMovementComponent->StartJog();
	if (bIsAiming) ExitAiming();
}

void ACMPCharacter::JogFinished(const FInputActionValue& Value)
{
	if (!Controller) return;

	CMPCharacterMovementComponent->StopJog();
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

void ACMPCharacter::SetHandTransform(const FTransform& NewTransform)
{
	HandTransform = NewTransform;

	if (HasAuthority()) OnRep_HandTransform();
}

void ACMPCharacter::SetInterpolateSight(const bool NewInterpolateSight)
{
	bInterpolateSight = NewInterpolateSight;
}

void ACMPCharacter::SetIsAiming(bool InIsAiming)
{
	if (!CurrentWeapon) return;

	bIsAiming = InIsAiming;
	CurrentWeapon->bIsAiming = bIsAiming;

	if (const auto AnimInstance = GetAnimInstance())
	{
		AnimInstance->bIsAiming = bIsAiming;
	}

	if (bIsAiming)
	{
		CMPCharacterMovementComponent->StopJog();
	}
	else if (!bIsWalkPressed)
	{
		// CMPCharacterMovementComponent->StopWalk();
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
		// CMPCharacterMovementComponent->GetCustomMovementState() == ECustomMovementState::ECMS_Run ||
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
	if (IsLocallyControlled()) return;

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
			OnRep_CurrentWeapon();
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

FTransform ACMPCharacter::GetRightHandTransform(ERelativeTransformSpace TransformSpace) const
{
	return GetMesh()->GetSocketTransform(RightHandSocketName, TransformSpace);
}

void ACMPCharacter::OnRep_ReplicatedAcceleration()
{
	if (!CMPCharacterMovementComponent) return;

	// Decompress Acceleration
	const double MaxAccel = CMPCharacterMovementComponent->MaxAcceleration;
	const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0;
	// [0, 255] -> [0, MaxAccel]
	const double AccelXYRadians = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;
	// [0, 255] -> [0, 2PI]

	FVector UnpackedAcceleration(FVector::ZeroVector);
	FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
	UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0;
	// [-127, 127] -> [-MaxAccel, MaxAccel]

	CMPCharacterMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
}

bool ACMPCharacter::UpdateSharedReplication()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FSharedRepMovement SharedMovement;
		if (SharedMovement.FillForCharacter(this))
		{
			// Only call FastSharedReplication if data has changed since the last frame.
			// Skipping this call will cause replication to reuse the same bunch that we previously
			// produced, but not send it to clients that already received. (But a new client who has not received
			// it, will get it this frame)
			if (!SharedMovement.Equals(LastSharedReplication, this))
			{
				LastSharedReplication = SharedMovement;
				ReplicatedMovementMode = SharedMovement.RepMovementMode;

				FastSharedReplication(SharedMovement);
			}
			return true;
		}
	}

	// We cannot fastrep right now. Don't send anything.
	return false;
}

void ACMPCharacter::FastSharedReplication_Implementation(const FSharedRepMovement& SharedRepMovement)
{
	if (GetWorld()->IsPlayingReplay())
	{
		return;
	}

	// Timestamp is checked to reject old moves.
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Timestamp
		ReplicatedServerLastTransformUpdateTimeStamp = SharedRepMovement.RepTimeStamp;

		// Movement mode
		if (ReplicatedMovementMode != SharedRepMovement.RepMovementMode)
		{
			ReplicatedMovementMode = SharedRepMovement.RepMovementMode;
			GetCharacterMovement()->bNetworkMovementModeChanged = true;
			GetCharacterMovement()->bNetworkUpdateReceived = true;
		}

		// Location, Rotation, Velocity, etc.
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
		MutableRepMovement = SharedRepMovement.RepMovement;

		// This also sets LastRepMovement
		OnRep_ReplicatedMovement();

		// Jump force
		bProxyIsJumpForceApplied = SharedRepMovement.bProxyIsJumpForceApplied;

		// Crouch
		if (bIsCrouched != SharedRepMovement.bIsCrouched)
		{
			bIsCrouched = SharedRepMovement.bIsCrouched;
			OnRep_IsCrouched();
		}
	}
}
