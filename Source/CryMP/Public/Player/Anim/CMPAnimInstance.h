// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CMPAnimInstance.generated.h"

/**
 * 
 */
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
