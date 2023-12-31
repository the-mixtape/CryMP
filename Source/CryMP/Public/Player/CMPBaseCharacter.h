// CryMP, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CMPBaseCharacter.generated.h"


class UCMPCharacterMovementComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UHandCombatComponent;


UCLASS()
class CRYMP_API ACMPBaseCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Movement)
	UCMPCharacterMovementComponent* CMPCharacterMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera)
	UCameraComponent* FPCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Components)
	UHandCombatComponent* HandCombatComponent;

public:
	ACMPBaseCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UFUNCTION(BlueprintCallable)
	void PlayNetworkMontage(UAnimMontage* AnimMontage);

	UFUNCTION(BlueprintCallable)
	void PlayMontage(UAnimMontage* AnimMontage);
	
	UFUNCTION(Server, Reliable)
	void ServerPlayMontage(UAnimMontage* AnimMontage);

	UFUNCTION(NetMulticast, Reliable)
	void MultiPlayMontage(UAnimMontage* AnimMontage);
	

#pragma region Input

private:
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Jog Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JogAction;

	/** Run Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RunAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HandAttackAction;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);
	
	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for jog input */
	void JogStarted(const FInputActionValue& Value);
	void JogFinished(const FInputActionValue& Value);
	
	/** Called for run input */
	void RunStarted(const FInputActionValue& Value);
	void RunFinished(const FInputActionValue& Value);

	/** Called for crouch input */
	void CrouchPressed(const FInputActionValue& Value);

	/** Called for jump input */
	void JumpPressed(const FInputActionValue& Value);
	void JumpReleased(const FInputActionValue& Value);

	/** Called for hand attack input */
	void HandAttackStarted(const FInputActionValue& Value);
#pragma endregion
};
