// Fill out your copyright notice in the Description page of Project Settings.


#include "Guns/MagParent.h"

#include "Net/UnrealNetwork.h"

void AMagParent::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		CurrentAmmo = MagSize + 1;
	}
}

void AMagParent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMagParent, CurrentAmmo, COND_OwnerOnly);
}
