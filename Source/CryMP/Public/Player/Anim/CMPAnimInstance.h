// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CMPAnimInstance.generated.h"


UENUM(BlueprintType)
enum class ELocomotionDirections : uint8
{
	ELD_Forward UMETA(DisplayName = "Fwd"),
	ELD_Backward UMETA(DisplayName = "Bwd"),
	ELD_Left UMETA(DisplayName = "Left"),
	ELD_Right UMETA(DisplayName = "Right"),
};


USTRUCT(BlueprintType)
struct FLocomotionDirectionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FMin;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FMax;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float bMin;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BMax;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeadZone;
	
};

USTRUCT(BlueprintType)
struct FDirectionAnims
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly);
	UAnimSequence* ForwardAnim;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly);
	UAnimSequence* BackwardAnim;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly);
	UAnimSequence* LeftAnim;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly);
	UAnimSequence* RightAnim;
	
};


UCLASS()
class CRYMP_API UCMPAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UCMPAnimInstance();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SwitchWeapon();

	UFUNCTION(BlueprintImplementableEvent)
	void NewHandTransform(const bool Interpolate, const FTransform& HandTransform);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Equip")
	UAnimSequence* EquipSequence;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Equip")
	UAnimSequence* UnEquipSequence;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Equip")
	FName EquipAnimSlotName = "DefaultSlot";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations")
	UAnimMontage* FireMontage;

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Base")
	bool bIsAiming;
	
};
