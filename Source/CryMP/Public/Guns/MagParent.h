// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Guns/GunPartParent.h"
#include "MagParent.generated.h"


UCLASS()
class CRYMP_API AMagParent : public AGunPartParent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Customization|Magazine")
	int MagSize = 30;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Customization|Magazine")
	int CurrentAmmo = 0;
	
};
