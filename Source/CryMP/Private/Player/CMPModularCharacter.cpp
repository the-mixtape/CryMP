// CryMP, All Rights Reserved.


#include "Player/CMPModularCharacter.h"

ACMPModularCharacter::ACMPModularCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Head = CreateDefaultSubobject<USkeletalMeshComponent>("Head");
	Head->SetupAttachment(GetMesh());
	Head->SetLeaderPoseComponent(GetMesh());
	Head->bOwnerNoSee = true;
	Head->bCastHiddenShadow = true;
	
	Hair = CreateDefaultSubobject<USkeletalMeshComponent>("Hair");
	Hair->SetupAttachment(GetMesh());
	Hair->SetLeaderPoseComponent(GetMesh());
	Hair->bOwnerNoSee = true;
	Hair->bCastHiddenShadow = true;
	
	Mask = CreateDefaultSubobject<USkeletalMeshComponent>("Mask");
	Mask->SetupAttachment(GetMesh());
	Mask->SetLeaderPoseComponent(GetMesh());
	Mask->bOwnerNoSee = true;
	Mask->bCastHiddenShadow = true;
	
	Jacket = CreateDefaultSubobject<USkeletalMeshComponent>("Jacket");
	Jacket->SetupAttachment(GetMesh());
	Jacket->SetLeaderPoseComponent(GetMesh());
	
	Vest = CreateDefaultSubobject<USkeletalMeshComponent>("Vest");
	Vest->SetupAttachment(GetMesh());
	Vest->SetLeaderPoseComponent(GetMesh());
	
	Legs = CreateDefaultSubobject<USkeletalMeshComponent>("Legs");
	Legs->SetupAttachment(GetMesh());
	Legs->SetLeaderPoseComponent(GetMesh());
	
	Shoes = CreateDefaultSubobject<USkeletalMeshComponent>("Shoes");
	Shoes->SetupAttachment(GetMesh());
	Shoes->SetLeaderPoseComponent(GetMesh());
}
