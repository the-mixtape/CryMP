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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACMPBaseCharacter::Move);
	
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACMPBaseCharacter::Look);
	
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ACMPBaseCharacter::SprintStarted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ACMPBaseCharacter::SprintFinished);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACMPBaseCharacter::SprintFinished);
	}
}

#pragma region Input
void ACMPBaseCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	
	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ACMPBaseCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
	
}

void ACMPBaseCharacter::SprintStarted(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		CMPCharacterMovementComponent->StartSprint();
	}
}

void ACMPBaseCharacter::SprintFinished(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		CMPCharacterMovementComponent->StopSprint();
	}
}
#pragma endregion 
