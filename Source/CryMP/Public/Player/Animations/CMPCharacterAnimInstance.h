// CryMP, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CMPCharacterAnimInstance.generated.h"


UENUM(BlueprintType)
enum EAnimOverlay : uint8
{
	EAO_Fists UMETA(DisplayName = "Fists"),
	EAO_Rifle UMETA(DisplayName = "Rifle"),
	EAO_Pistol UMETA(DisplayName = "Pistol")
};


UCLASS()
class CRYMP_API UCMPCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
};
