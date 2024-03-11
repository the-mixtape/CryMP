// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CMPCharacter.generated.h"

class UCMPAnimInstance;
class UCameraComponent;
class UCMPCharacterMovementComponent;
class UInputMappingContext;
class UInputAction;
class AGunParent;
class AGunPartParent;


UCLASS()
class CRYMP_API ACMPCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Movement)
	UCMPCharacterMovementComponent* CMPCharacterMovementComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Camera)
	UCameraComponent* FPCamera;

public:
	ACMPCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;


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

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;
	
	/** Run Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RunAction;
	
	/** Run Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for sprint input */
	void AimStarted(const FInputActionValue& Value);
	void AimFinished(const FInputActionValue& Value);
	
	/** Called for sprint input */
	void RunStarted(const FInputActionValue& Value);
	void RunFinished(const FInputActionValue& Value);

	/** Called for walk input */
	void WalkStarted(const FInputActionValue& Value);
	void WalkFinished(const FInputActionValue& Value);

	/** Called for crouch input */
	void CrouchPressed(const FInputActionValue& Value);

	/** Called for jump input */
	void JumpPressed(const FInputActionValue& Value);
	void JumpReleased(const FInputActionValue& Value);

private:
	bool bIsWalkPressed;
#pragma endregion

#pragma region Weapon
public:
	UPROPERTY(BlueprintReadOnly, Category="Customization|Weapon|Sway")
	float InputAxisYaw;
	
	UPROPERTY(BlueprintReadOnly, Category="Customization|Weapon|Sway")
	float InputAxisPitch;
	
	UPROPERTY(BlueprintReadOnly, Category="Customization|Weapon|Sway")
	float InputAxisRight;

	UPROPERTY(BlueprintReadOnly, Category="Customization|Weapon|Aiming")
	bool bIsHoldToAim = true;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Customization")
	TMap<TSubclassOf<AGunParent>, uint8> StartingGunsInventory;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Sockets")
	FName GunHolsterSocketName = "GunHolster";
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon|Sockets")
	FName GunSocketName = "Gun";

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentWeapon)
	AGunParent* CurrentWeapon;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HandTransform)
	FTransform HandTransform;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bInterpolateSight;
	
private:
	UPROPERTY()
	TArray<AGunParent*> GunsInventory;
	
	UPROPERTY()
	bool bIsAiming;

	void SetIsAiming(bool InIsAiming);

	UPROPERTY()
	AGunParent* LastWeapon;

	UFUNCTION()
	void OnRep_CurrentWeapon();

	UFUNCTION()
	void OnRep_HandTransform();

private:
	void EnterAiming();
	void ExitAiming();

	UFUNCTION(Server, Reliable)
	void ServerEnterAiming();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEnterAiming();

	UFUNCTION(Server, Reliable)
	void ServerExitAiming();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastExitAiming();
	
public:
	UFUNCTION(BlueprintCallable)
	void SwitchWeapon();

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsWeaponEquipped() const { return CurrentWeapon != nullptr; }
	
private:
	void SpawnGunsInventory();
	void AddToGunInventory(AGunParent* Gun);
	void UnEquipLastWeapon();
	void EquipCurrentWeapon();

	void UpdateWeaponAnims();
#pragma endregion

#pragma region Animations
protected:
	UPROPERTY(BlueprintReadOnly, Category=Animactions)
	UCMPAnimInstance* CMPAnimInstance;
#pragma endregion

	UCMPAnimInstance* GetAnimInstance();
};
