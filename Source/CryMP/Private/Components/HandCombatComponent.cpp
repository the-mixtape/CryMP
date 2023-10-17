// CryMP, All Rights Reserved.


#include "Components/HandCombatComponent.h"

#include "CMPBaseCharacter.h"

UHandCombatComponent::UHandCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHandCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACMPBaseCharacter>(GetOwner());
}

void UHandCombatComponent::PressedAttack()
{
	const float CurrentTime = GetWorld()->GetRealTimeSeconds();
	const float DelayDuration = CurrentTime - LastAttackTime;

	if (DelayDuration < Delay) return;

	if (DelayDuration > ResetComboTime)
	{
		ComboCount = 0;
	}
	else
	{
		IncrementComboCount();
	}

	const FHandCombatAttack Attack = Attacks[ComboCount];
	DoAttack(Attack);
}

void UHandCombatComponent::IncrementComboCount()
{
	ComboCount++;

	if (ComboCount >= Attacks.Num())
	{
		ComboCount = 0;
	}
}

void UHandCombatComponent::DoAttack(const FHandCombatAttack& Attack)
{
	LastAttackTime = GetWorld()->GetRealTimeSeconds();
	Delay = Attack.AttackTime;
	OwnerCharacter->PlayNetworkMontage(Attack.AnimMontage);
}
