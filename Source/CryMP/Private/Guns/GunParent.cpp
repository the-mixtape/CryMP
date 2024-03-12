// Fill out your copyright notice in the Description page of Project Settings.


#include "Guns/GunParent.h"

#include "CMPCharacter.h"
#include "MagParent.h"
#include "Guns/GunPartParent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AGunParent::AGunParent()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	WeaponMesh->SetupAttachment(GetRootComponent());

	AvailableFireModes.Add(EFireModes::EFM_Safe);
	AvailableFireModes.Add(EFireModes::EFM_Semi);
	AvailableFireModes.Add(EFireModes::EFM_Burst);
	AvailableFireModes.Add(EFireModes::EFM_Auto);
}

void AGunParent::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ResetGunProperties();
		AddStartingParts();
		ModifyGunProperties();
		GenerateSights();
		SetMagazine();

		CharacterOwner = Cast<ACMPCharacter>(GetOwner());
	}

	SetStartingFireMode();
}

void AGunParent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGunParent, SpreadAiming, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGunParent, SpreadHip, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGunParent, RecoilPerShot, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AGunParent, StartingParts, COND_InitialOnly);
	DOREPLIFETIME(AGunParent, Magazine);
}

void AGunParent::Equip()
{
}

void AGunParent::UnEquip()
{
}

void AGunParent::CalculateHandTransforms()
{
}

void AGunParent::SetStartingFireMode()
{
	if (!GetInstigator() || !GetInstigator()->IsLocallyControlled()) return;

	if (AvailableFireModes.Find(StartingFireMode) != INDEX_NONE)
	{
		CurrentFireMode = StartingFireMode;
		return;
	}

	const auto ModesNum = AvailableFireModes.Num();
	CurrentFireMode = AvailableFireModes.GetData()[ModesNum];
}

void AGunParent::ResetGunProperties()
{
	SpreadAiming = DefaultSpreadAiming;
	SpreadHip = DefaultSpreadHip;
	RecoilPerShot = DefaultRecoilPerShot;
}

void AGunParent::AddStartingParts()
{
	for (const auto& Part : StartingParts)
	{
		AActor* BaseActor = this;
		if (Part.BaseIndex != -1)
		{
			BaseActor = CurrentParts.GetData()[Part.BaseIndex];
		}

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.TransformScaleMethod = ESpawnActorScaleMethod::SelectDefaultAtRuntime;

		const auto GunPart = GetWorld()->SpawnActor<AGunPartParent>(
			Part.PartClass.Get(),
			FVector::ZeroVector, FRotator::ZeroRotator, Params);

		CustomizePart(GunPart, Part.Actioned);

		const FAttachmentTransformRules Rules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepRelative,
			true);

		GunPart->AttachToComponent(WeaponMesh, Rules, Part.BaseSocket);
		CurrentParts.AddUnique(GunPart);

		FName TopSocket = "None";
		UMeshComponent* MeshComp;
		GunPart->FindTopSocket(TopSocket, MeshComp);

		const auto GunPartMesh = GunPart->GetComponentByClass<UMeshComponent>();
		AdjustRotation(GunPart, GunPartMesh, TopSocket);
		AdjustLocation(GunPart, GunPartMesh, TopSocket, WeaponMesh, Part.BaseSocket);
	}
}

void AGunParent::ModifyGunProperties()
{
	const auto GunParts = GetAttachedPartsRecursevely();
	for (const auto& GunPart : GunParts)
	{
		SpreadAiming += GunPart->AddSpreadAiming;
		SpreadHip += GunPart->AddSpreadHip;
		RecoilPerShot += GunPart->AddRecoilPerShot;
	}
}

void AGunParent::GenerateSights()
{
	Sights.Reset();
	CurrentSight = 0;

	TArray<AAssemblableParent*> AllParts;
	GetAllParts(AllParts);

	for (const auto Part : AllParts)
	{
		GenerateOpticSights(Part);
		GenerateIronSights(Part, AllParts);
	}
}

void AGunParent::SetMagazine()
{
	for (const auto Part : CurrentParts)
	{
		if (const auto Mag = Cast<AMagParent>(Part))
		{
			Magazine = Mag;
			return;
		}
	}

	Magazine = nullptr;
}

void AGunParent::CustomizePart(AGunPartParent* Part, bool IsCustomActioned)
{
	if (!Part->bHasCustomAction) return;

	Part->bIsCustomActioned = IsCustomActioned;
	Part->CustomAction(IsCustomActioned);
}

void AGunParent::AdjustRotation(AActor* Part, const UMeshComponent* PartComponent, const FName& TopSocket)
{
	const auto ComponentRotate = PartComponent->GetSocketTransform(TopSocket, RTS_Actor).Rotator();
	Part->AddActorLocalRotation(UKismetMathLibrary::NegateRotator(ComponentRotate));
}

void AGunParent::AdjustLocation(AActor* Part, const UMeshComponent* PartComponent, const FName& TopSocket,
                                const UMeshComponent* ParentMeshComponent, const FName& BaseSocket)
{
	const auto ParentTransform = ParentMeshComponent->GetSocketTransform(BaseSocket);
	const auto PartMeshSocketLocation = PartComponent->GetSocketLocation(TopSocket);

	const auto InverseTransformLocation = UKismetMathLibrary::InverseTransformLocation(
		ParentTransform, PartMeshSocketLocation);

	const auto NewLocation = UKismetMathLibrary::TransformLocation(
		ParentTransform, UKismetMathLibrary::NegateVector(InverseTransformLocation));
	
	Part->SetActorLocation(NewLocation);
}

void AGunParent::GenerateOpticSights(AAssemblableParent* Part)
{
	for (const auto& OpticAimPoint : Part->OpticAimPoints)
	{
		const bool bIsAimPointWithinMaxAngle = IsAimPointWithinMaxAngle(OpticAimPoint.MeshComponent,
		                                                                OpticAimPoint.Socket);
		if (bIsAimPointWithinMaxAngle && OpticAimPoint.bInUse)
		{
			FSightData SightData;
			SightData.SightType = ESightTypes::EST_Optic;
			SightData.OpticOrFrontSocket = OpticAimPoint.Socket;
			SightData.OpticOrFrontComponent = OpticAimPoint.MeshComponent;

			Sights.AddUnique(SightData);
		}
	}
}

void AGunParent::GenerateIronSights(AAssemblableParent* InPart, const TArray<AAssemblableParent*>& AllParts)
{
	for (const auto FrontAimPoint : InPart->FrontAimPoints)
	{
		const bool bIsAimPointWithinMaxAngle = IsAimPointWithinMaxAngle(FrontAimPoint.MeshComponent,
		                                                                FrontAimPoint.Socket);
		if (bIsAimPointWithinMaxAngle && FrontAimPoint.bInUse)
		{
			for (const auto Part : AllParts)
			{
				bool bNeedBreak = false;
				for (const auto RearAimPoint : Part->RearAimPoints)
				{
					if (!RearAimPoint.bInUse) continue;
					if (IronSightsCanBePaired(RearAimPoint.Socket, RearAimPoint.MeshComponent,
					                          FrontAimPoint.Socket, FrontAimPoint.MeshComponent))
					{
						FSightData SightData;
						SightData.SightType = ESightTypes::EST_IronSight;
						SightData.OpticOrFrontSocket = FrontAimPoint.Socket;
						SightData.OpticOrFrontComponent = FrontAimPoint.MeshComponent;
						SightData.RearSocket = RearAimPoint.Socket;
						SightData.RearComponent = RearAimPoint.MeshComponent;

						Sights.AddUnique(SightData);

						bNeedBreak = true;
					}
				}

				if (bNeedBreak) break;
			}
		}
	}
}

bool AGunParent::IronSightsCanBePaired(FName RearSocket, UMeshComponent* RearComp,
                                       FName FrontSocket, UMeshComponent* FrontComp)
{
	const auto RearLocation = RearComp->GetSocketLocation(RearSocket);
	const auto FrontTransform = FrontComp->GetSocketTransform(FrontSocket);
	const auto RearFrontLookRotation = UKismetMathLibrary::FindLookAtRotation(
		RearLocation, FrontTransform.GetLocation());
	const auto InverseRotation = UKismetMathLibrary::InverseTransformRotation(FrontTransform, RearFrontLookRotation);
	return InverseRotation.Pitch > -5.f && InverseRotation.Pitch < 5.f &&
		(InverseRotation.Yaw > 178.f || InverseRotation.Yaw < -178.f);
}

bool AGunParent::IsAimPointWithinMaxAngle(UMeshComponent* MeshComponent, FName SocketName) const
{
	const auto SocketRotation = MeshComponent->GetSocketRotation(SocketName);
	const auto SocketUpVector = UKismetMathLibrary::GetUpVector(SocketRotation);
	const auto WeaponUpVector = WeaponMesh->GetUpVector();

	const auto VectorsDot = UKismetMathLibrary::Dot_VectorVector(SocketUpVector, WeaponUpVector);
	return UKismetMathLibrary::DegAcos(VectorsDot) < SightMaxAngle;
}

void AGunParent::GetAllParts(TArray<AAssemblableParent*>& AllParts) const
{
	TArray<UPrimitiveComponent*> AllPrimitiveComponents;
	AllPrimitiveComponents.AddUnique(WeaponMesh);

	TArray<USceneComponent*> MeshChilds;
	WeaponMesh->GetChildrenComponents(true, MeshChilds);

	for (const auto MeshChild : MeshChilds)
	{
		if (const auto PrimComp = Cast<UPrimitiveComponent>(MeshChild))
		{
			AllPrimitiveComponents.AddUnique(PrimComp);
		}
	}

	TArray<AActor*> Actors;
	UKismetSystemLibrary::GetActorListFromComponentList(AllPrimitiveComponents, AAssemblableParent::StaticClass(),
	                                                    Actors);

	for (const auto Actor : Actors)
	{
		if (const auto Part = Cast<AAssemblableParent>(Actor))
		{
			AllParts.AddUnique(Part);
		}
	}
}

TArray<AGunPartParent*> AGunParent::GetAttachedPartsRecursevely()
{
	TArray<AGunPartParent*> Parts;
	GetAttachedInternalLoop(this, Parts);
	return Parts;
}

void AGunParent::GetAttachedInternalLoop(const AAssemblableParent* Part, TArray<AGunPartParent*>& Parts) const
{
	TArray<AActor*> Actors;
	Part->GetAttachedActors(Actors);
	for (const auto Actor : Actors)
	{
		if (auto GunPart = Cast<AGunPartParent>(Actor))
		{
			Parts.AddUnique(GunPart);
			GetAttachedInternalLoop(GunPart, Parts);
		}
	}
}

void AGunParent::OnRep_CurrentParts()
{
}
