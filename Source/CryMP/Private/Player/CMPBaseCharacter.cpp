// CryMP, All Rights Reserved.


#include "Player/CMPBaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Player/CMPCharacterMovementComponent.h"

ACMPBaseCharacter::ACMPBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCMPCharacterMovementComponent>(CharacterMovementComponentName)) 
{
	PrimaryActorTick.bCanEverTick = true;

	CMPCharacterMovementComponent = Cast<UCMPCharacterMovementComponent>(GetCharacterMovement());

	FPCamera = CreateDefaultSubobject<UCameraComponent>("FPCamera");
	FPCamera->SetupAttachment(GetMesh(), "CameraSocket");
	FPCamera->bUsePawnControlRotation = true;

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Legs");
	LegsMesh->SetupAttachment(GetMesh());
	LegsMesh->SetLeaderPoseComponent(GetMesh());
}

void ACMPBaseCharacter::BeginPlay()
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
}

void ACMPBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACMPBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACMPBaseCharacter::JumpPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACMPBaseCharacter::JumpReleased);
	
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACMPBaseCharacter::Move);
	
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACMPBaseCharacter::Look);
	
		EnhancedInputComponent->BindAction(JogAction, ETriggerEvent::Started, this, &ACMPBaseCharacter::JogStarted);
		EnhancedInputComponent->BindAction(JogAction, ETriggerEvent::Canceled, this, &ACMPBaseCharacter::JogFinished);
		EnhancedInputComponent->BindAction(JogAction, ETriggerEvent::Completed, this, &ACMPBaseCharacter::JogFinished);

		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ACMPBaseCharacter::RunStarted);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Canceled, this, &ACMPBaseCharacter::RunFinished);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ACMPBaseCharacter::RunFinished);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ACMPBaseCharacter::CrouchPressed);
	}
}

#pragma region Input
void ACMPBaseCharacter::Move(const FInputActionValue& Value)
{
	if (!Controller) return;
	
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	AddMovementInput(GetActorRightVector(), MovementVector.X);
}

void ACMPBaseCharacter::Look(const FInputActionValue& Value)
{
	if (!Controller) return;
	
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ACMPBaseCharacter::JogStarted(const FInputActionValue& Value)
{
	if (!Controller) return;
	
	if(CMPCharacterMovementComponent->IsCrouching())
	{
		CMPCharacterMovementComponent->StopCrouch();
		return;
	}
	
	CMPCharacterMovementComponent->StartJog();
}

void ACMPBaseCharacter::JogFinished(const FInputActionValue& Value)
{
	if (!Controller) return;
	
	CMPCharacterMovementComponent->StopJog();
}

void ACMPBaseCharacter::RunStarted(const FInputActionValue& Value)
{
	if (!Controller) return;
	
	if(CMPCharacterMovementComponent->IsCrouching())
	{
		CMPCharacterMovementComponent->StopCrouch();
		return;
	}
	
	CMPCharacterMovementComponent->StartRun();
}

void ACMPBaseCharacter::RunFinished(const FInputActionValue& Value)
{
	if (!Controller) return;
	
	CMPCharacterMovementComponent->StopRun();
}

void ACMPBaseCharacter::CrouchPressed(const FInputActionValue& Value)
{
	if (!Controller) return;
	
	CMPCharacterMovementComponent->ToggleCrouch();
}

void ACMPBaseCharacter::JumpPressed(const FInputActionValue& Value)
{
	if(CMPCharacterMovementComponent->IsCrouching())
	{
		CMPCharacterMovementComponent->StopCrouch();
		return;
	}
	
	ACharacter::Jump();
}

void ACMPBaseCharacter::JumpReleased(const FInputActionValue& Value)
{
	ACharacter::StopJumping();
}
#pragma endregion 
