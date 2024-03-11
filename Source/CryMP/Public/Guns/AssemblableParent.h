// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AssemblableParent.generated.h"

USTRUCT(BlueprintType)
struct FAimPoint
{
	GENERATED_BODY()

	UPROPERTY()
	FName Socket;

	UPROPERTY()
	UMeshComponent* MeshComponent;

	UPROPERTY()
	bool bInUse;

	inline bool operator==(const FAimPoint& other) const
	{
		return other.Socket == this->Socket &&
			other.MeshComponent == this->MeshComponent &&
			other.bInUse == this->bInUse;
	}
};

UCLASS()
class CRYMP_API AAssemblableParent : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category=Components)
	USceneComponent* DefaultSceneRoot;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category=Prefixs)
	FString AimPointOpticPrefix = "AimPoint_Optic_";

	UPROPERTY(EditDefaultsOnly, Category=Prefixs)
	FString AimPointFrontPrefix = "AimPoint_Front_";

	UPROPERTY(EditDefaultsOnly, Category=Prefixs)
	FString AimPointRearPrefix = "AimPoint_Rear_";

public:
	AAssemblableParent();

	UFUNCTION(Server, Reliable)
	void ServerAction();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void GenerateAimPoints();

public:
	virtual void CustomAction(bool IsCustomActioned);

public:
	UFUNCTION()
	void ApplyMaterial(UMaterialInterface* NewMaterial);

public:
	UPROPERTY()
	TArray<FAimPoint> OpticAimPoints;

	UPROPERTY()
	TArray<FAimPoint> FrontAimPoints;

	UPROPERTY()
	TArray<FAimPoint> RearAimPoints;

	UPROPERTY(ReplicatedUsing=OnRep_Material)
	UMaterialInterface* Material;

protected:
	UFUNCTION()
	void OnRep_Material();

	UFUNCTION()
	void OnRep_IsCustomActioned();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Customization|CustomAction")
	bool bHasCustomAction;

	UPROPERTY(ReplicatedUsing=OnRep_IsCustomActioned)
	bool bIsCustomActioned;
};
