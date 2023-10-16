// CryMP, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/CMPBaseCharacter.h"
#include "CMPModularCharacter.generated.h"

/**
 * 
 */
UCLASS()
class CRYMP_API ACMPModularCharacter : public ACMPBaseCharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Meshes)
	USkeletalMeshComponent* Head;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Meshes)
	USkeletalMeshComponent* Hair;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Meshes)
	USkeletalMeshComponent* Mask;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Meshes)
	USkeletalMeshComponent* Jacket;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Meshes)
	USkeletalMeshComponent* Vest;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Meshes)
	USkeletalMeshComponent* Legs;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Meshes)
	USkeletalMeshComponent* Shoes;

public:
	ACMPModularCharacter(const FObjectInitializer& ObjectInitializer);
	
};
