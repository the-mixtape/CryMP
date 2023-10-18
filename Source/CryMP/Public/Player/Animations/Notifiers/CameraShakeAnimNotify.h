// CryMP, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CameraShakeAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class CRYMP_API UCameraShakeAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly, Category=Params)
	TSubclassOf<class UCameraShakeBase> Shake;
	
	UPROPERTY(EditInstanceOnly, Category=Params)
	float Scale = 1.f;
	
	UPROPERTY(EditInstanceOnly, Category=Params)
	ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal;
	
	UPROPERTY(EditInstanceOnly, Category=Params)
	FRotator UserPlaySpaceRot = FRotator::ZeroRotator;
	
protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
};
