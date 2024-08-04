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

/**
 * FCMPReplicatedAcceleration: Compressed representation of acceleration
 */
USTRUCT()
struct FCMPReplicatedAcceleration
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 AccelXYRadians = 0;	// Direction of XY accel component, quantized to represent [0, 2*pi]

	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	UPROPERTY()
	int8 AccelZ = 0;	// Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};


/** The type we use to send FastShared movement updates. */
USTRUCT()
struct FSharedRepMovement
{
	GENERATED_BODY()

	FSharedRepMovement();

	bool FillForCharacter(ACharacter* Character);
	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	UPROPERTY(Transient)
	FRepMovement RepMovement;

	UPROPERTY(Transient)
	float RepTimeStamp = 0.0f;

	UPROPERTY(Transient)
	uint8 RepMovementMode = 0;

	UPROPERTY(Transient)
	bool bProxyIsJumpForceApplied = false;

	UPROPERTY(Transient)
	bool bIsCrouched = false;
};

template<>
struct TStructOpsTypeTraits<FSharedRepMovement> : public TStructOpsTypeTraitsBase2<FSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

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
	UFUNCTION(BlueprintPure, Category=Camera)
	FORCEINLINE UCameraComponent* GetFPCamera() const { return FPCamera; }

public:
	ACMPCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
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
	UInputAction* JogAction;

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
	void JogStarted(const FInputActionValue& Value);
	void JogFinished(const FInputActionValue& Value);

	/** Called for crouch input */
	void CrouchPressed(const FInputActionValue& Value);

	/** Called for jump input */
	void JumpPressed(const FInputActionValue& Value);
	void JumpReleased(const FInputActionValue& Value);
#pragma endregion
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

public:
	UFUNCTION(BlueprintPure, Category=Weapon)
	FORCEINLINE AGunParent* GetCurrentWeapon() const { return CurrentWeapon; }

protected:
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bInterpolateSight;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HandTransform)
	FTransform HandTransform;

public:
	void SetHandTransform(const FTransform& NewTransform);
	void SetInterpolateSight(const bool NewInterpolateSight);
	
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

protected:
	UPROPERTY(EditDefaultsOnly, Category="Customization|Sockets")
	FName RightHandSocketName = "hand_r";

public:
	FTransform GetRightHandTransform(ERelativeTransformSpace TransformSpace) const;

private:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FCMPReplicatedAcceleration ReplicatedAcceleration;

private:
	UFUNCTION()
	void OnRep_ReplicatedAcceleration();

public:
	
	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	UFUNCTION(NetMulticast, unreliable)
	void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;

	virtual bool UpdateSharedReplication();
};
