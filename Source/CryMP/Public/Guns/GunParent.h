// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Guns/AssemblableParent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GunParent.generated.h"


class AGunPartParent;
class AMagParent;
class ACMPCharacter;

struct FAutoAttachPart;


UENUM(BlueprintType)
enum class EFireModes : uint8
{
	EFM_Safe UMETA(DisplayName="Safe"),
	EFM_Semi UMETA(DisplayName="Semi"),
	EFM_Burst UMETA(DisplayName="Burst"),
	EFM_Auto UMETA(DisplayName="Auto"),
};


UENUM(BlueprintType)
enum class ESightTypes : uint8
{
	EST_Optic UMETA(DisplayName="Optic"),
	EST_IronSight UMETA(DisplayName="IronSight"),
};


USTRUCT(BlueprintType)
struct FSightData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ESightTypes SightType = ESightTypes::EST_IronSight;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName OpticOrFrontSocket = "None";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMeshComponent* OpticOrFrontComponent = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName RearSocket = "None";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMeshComponent* RearComponent = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FTransform HandTransform;

	inline bool operator==(const FSightData& other) const
	{
		return other.SightType == this->SightType &&
			other.OpticOrFrontSocket == this->OpticOrFrontSocket &&
			other.OpticOrFrontComponent == this->OpticOrFrontComponent &&
			other.RearSocket == this->RearSocket &&
			other.RearComponent == this->RearComponent &&
			UKismetMathLibrary::EqualEqual_TransformTransform(other.HandTransform, this->HandTransform);
	}
};

UCLASS(BlueprintType)
class CRYMP_API AGunParent : public AAssemblableParent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category=Components)
	USkeletalMeshComponent* WeaponMesh;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Customization|Fire")
	TArray<EFireModes> AvailableFireModes;

	UPROPERTY(EditDefaultsOnly, Category="Customization|Fire")
	EFireModes StartingFireMode;

	UPROPERTY(BlueprintReadOnly, Category="Customization|Fire")
	EFireModes CurrentFireMode;

	UPROPERTY(BlueprintReadOnly, Replicated)
	ACMPCharacter* CharacterOwner;

public:
	UPROPERTY(EditDefaultsOnly, Category="Animations")
	TSubclassOf<UAnimInstance> AnimLayer;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Equip")
	UAnimSequence* EquipSequence;

	UPROPERTY(EditDefaultsOnly, Category="Animations|Equip")
	UAnimSequence* UnEquipSequence;

	UPROPERTY(EditDefaultsOnly, Category="Animations")
	UAnimMontage* FireMontage;

public:
	AGunParent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void Equip();
	void UnEquip();

	void CalculateHandTransforms();

private:
	void SetStartingFireMode();

public:
	UPROPERTY(BlueprintReadOnly)
	bool bIsAiming;

protected:
	UPROPERTY(BlueprintReadOnly, Replicated, Category="Spread")
	float SpreadAiming;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Spread")
	float SpreadHip;

	UPROPERTY(EditDefaultsOnly, Category="Customization|Spread")
	float DefaultSpreadAiming = 0.01f;

	UPROPERTY(EditDefaultsOnly, Category="Customization|Spread")
	float DefaultSpreadHip = 1;

	UPROPERTY(BlueprintReadOnly, Replicated, Category="Recoil")
	float RecoilPerShot;

	UPROPERTY(EditDefaultsOnly, Category="Customization|Recoil")
	float DefaultRecoilPerShot = -0.3f;

	/**
	* -If base index is -1,
	* the base for this part (the part on which this part is mounted) is the main gun part (the gun actor itself).
	* -If base index is >= 0,
	* this index indicates the element in the StartingParts array that will be the base for this part
	 */
	UPROPERTY(EditDefaultsOnly, Replicated, Category="Customization|StartingParts")
	TArray<FAutoAttachPart> StartingParts;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentParts, BlueprintReadOnly, Category="Parts")
	TArray<AGunPartParent*> CurrentParts;

	UPROPERTY(BlueprintReadOnly, Category="Sights")
	TArray<FSightData> Sights;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Sights")
	AMagParent* Magazine;

	UPROPERTY(BlueprintReadOnly, Category="Sights")
	int CurrentSight;

	UPROPERTY(EditDefaultsOnly, Replicated, Category="Customization|Aiming")
	float SightMaxAngle = 50;

private:
	void ResetGunProperties();
	void AddStartingParts();
	void ModifyGunProperties();
	void GenerateSights();
	void SetMagazine();

	static void CustomizePart(AGunPartParent* Part, bool IsCustomActioned);

	static void AdjustRotation(AActor* Part, const UMeshComponent* PartComponent, const FName& TopSocket);
	static void AdjustLocation(AActor* Part, const UMeshComponent* PartComponent, const FName& TopSocket,
	                           const UMeshComponent* ParentMeshComponent, const FName& BaseSocket);
	
	void GenerateOpticSights(AAssemblableParent* Part);
	void GenerateIronSights(AAssemblableParent* InPart, const TArray<AAssemblableParent*>& AllParts);
	
	static bool IronSightsCanBePaired(FName RearSocket, UMeshComponent* RearComp, FName FrontSocket,
	                                  UMeshComponent* FrontComp);

	bool IsAimPointWithinMaxAngle(UMeshComponent* MeshComponent, FName SocketName) const;

	void GetAllParts(TArray<AAssemblableParent*>& AllParts) const;
	TArray<AGunPartParent*> GetAttachedPartsRecursevely();

	void GetAttachedInternalLoop(const AAssemblableParent* Part, TArray<AGunPartParent*>& Parts) const;

	UFUNCTION()
	void OnRep_CurrentParts();
};
