// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CMPCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EGaits : uint8
{
	ECMS_Walk UMETA(DisplayName = "Walk"),
	ECMS_Jog UMETA(DisplayName = "Jog"),
	ECMS_Run UMETA(DisplayName = "Run")
};

USTRUCT(BlueprintType)
struct FGaitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAcceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDeceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingFrictionFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseSeparateBrakingFriction;
};


UCLASS()
class CRYMP_API UCMPCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	typedef UCharacterMovementComponent Super;

	class FSavedMove_CMP : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;

		uint8 Saved_bWantsToRun:1;
		uint8 Saved_bWantsToWalk:1;

	public:
		FSavedMove_CMP();

		virtual bool
		CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
		                        class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_CMP : public FNetworkPredictionData_Client_Character
	{
		typedef FNetworkPredictionData_Client_Character Super;

	public:
		FNetworkPredictionData_Client_CMP(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	bool Safe_bWantsToRun;
	bool Safe_bWantsToWalk;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FGaitSettings RunSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FGaitSettings JogSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	FGaitSettings WalkSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Settings")
	float Run_Angle = 60.f;

	UPROPERTY(Replicated)
	EGaits CurrentGait;

public:
	UCMPCharacterMovementComponent();

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void ReplicateMovementState();

public:
	UFUNCTION(BlueprintCallable)
	void StartRun();
	UFUNCTION(BlueprintCallable)
	void StopRun();

	UFUNCTION(BlueprintCallable)
	void StartWalk();
	UFUNCTION(BlueprintCallable)
	void StopWalk();

	UFUNCTION(BlueprintCallable)
	void ToggleCrouch();
	UFUNCTION(BlueprintCallable)
	void StartCrouch();
	UFUNCTION(BlueprintCallable)
	void StopCrouch();

	UFUNCTION(BlueprintPure)
	FORCEINLINE EGaits GetCurrentGait() const
	{
		return CurrentGait;
	}

	FORCEINLINE void UseGaitSettings(const FGaitSettings& Settings)
	{
		MaxWalkSpeed = Settings.MaxWalkSpeed;
		MaxAcceleration = Settings.MaxAcceleration;
		BrakingDecelerationWalking = Settings.BrakingDeceleration;
		BrakingFrictionFactor = Settings.BrakingFrictionFactor;
		bUseSeparateBrakingFriction = Settings.bUseSeparateBrakingFriction;
		BrakingFriction = Settings.BrakingFriction;
	}
};
