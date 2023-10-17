// CryMP, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HandCombatComponent.generated.h"


class ACMPBaseCharacter;


USTRUCT(BlueprintType)
struct FHandCombatAttack
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* AnimMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackTime;
	
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CRYMP_API UHandCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHandCombatComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void PressedAttack();
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Animations)
	TArray<FHandCombatAttack> Attacks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Settings)
	float ResetComboTime = 1.f;

private:
	void IncrementComboCount();
	void DoAttack(const FHandCombatAttack& Attack);
	
private:
	UPROPERTY()
	ACMPBaseCharacter* OwnerCharacter;

	float LastAttackTime = 0.f;
	float Delay = 0.f;

	uint8 ComboCount = -1;
};
