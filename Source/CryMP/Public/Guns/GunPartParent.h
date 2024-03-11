// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Guns/AssemblableParent.h"
#include "GunPartParent.generated.h"


class AGunPartParent;


USTRUCT(BlueprintType)
struct FAutoAttachPart
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AGunPartParent> PartClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int BaseIndex = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName BaseSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool Actioned;
};


UCLASS(BlueprintType)
class CRYMP_API AGunPartParent : public AAssemblableParent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Customization)
	float AddSpreadHip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Customization)
	float AddSpreadAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Customization)
	float AddRecoilPerShot;

protected:
	UPROPERTY(EditDefaultsOnly, Category=Prefixs)
	FString TopPrefix = "Top_";

public:
	void GetAttachedInternalLoop(const AAssemblableParent* Part, TArray<AGunPartParent*>& Parts) const;

	void FindTopSocket(FName& SocketName, UMeshComponent*& MeshComponent) const;
};
