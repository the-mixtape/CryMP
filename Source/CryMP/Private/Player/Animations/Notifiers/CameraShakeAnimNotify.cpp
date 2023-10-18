// CryMP, All Rights Reserved.


#include "Player/Animations/Notifiers/CameraShakeAnimNotify.h"

#include "GameFramework/Character.h"

void UCameraShakeAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if(const auto Character = Cast<ACharacter>(MeshComp->GetOwner()))
	{
		if(const auto PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			PlayerController->ClientStartCameraShake(Shake, Scale, PlaySpace, UserPlaySpaceRot);
		}
	}
}
