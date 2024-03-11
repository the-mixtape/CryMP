// Fill out your copyright notice in the Description page of Project Settings.


#include "Guns/AssemblableParent.h"

#include "Kismet/KismetStringLibrary.h"
#include "Net/UnrealNetwork.h"

AAssemblableParent::AAssemblableParent()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>("DefaultSceneRoot");
	SetRootComponent(DefaultSceneRoot);
}

void AAssemblableParent::ServerAction_Implementation()
{
	bIsCustomActioned = !bIsCustomActioned;
}

void AAssemblableParent::BeginPlay()
{
	Super::BeginPlay();

	GenerateAimPoints();
}

void AAssemblableParent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAssemblableParent, Material);
	DOREPLIFETIME(AAssemblableParent, bIsCustomActioned);
}

void AAssemblableParent::GenerateAimPoints()
{
	if (!HasAuthority()) return;

	const auto MeshComponent = GetComponentByClass<UMeshComponent>();
	const auto NameOfSockets = MeshComponent->GetAllSocketNames();
	for (const auto SocketName : NameOfSockets)
	{
		if (UKismetStringLibrary::StartsWith(SocketName.ToString(), AimPointOpticPrefix, ESearchCase::IgnoreCase))
		{
			OpticAimPoints.AddUnique(FAimPoint(SocketName, MeshComponent, true));
		}
		else if (UKismetStringLibrary::StartsWith(SocketName.ToString(), AimPointFrontPrefix, ESearchCase::IgnoreCase))
		{
			FrontAimPoints.AddUnique(FAimPoint(SocketName, MeshComponent, true));
		}
		else if (UKismetStringLibrary::StartsWith(SocketName.ToString(), AimPointRearPrefix, ESearchCase::IgnoreCase))
		{
			RearAimPoints.AddUnique(FAimPoint(SocketName, MeshComponent, true));
		}
	}
}

void AAssemblableParent::CustomAction(bool IsCustomActioned)
{
	
}

void AAssemblableParent::ApplyMaterial(UMaterialInterface* NewMaterial)
{
	Material = NewMaterial;
}

void AAssemblableParent::OnRep_Material()
{
	GetComponentByClass<UMeshComponent>()->SetMaterial(0, Material);
}

void AAssemblableParent::OnRep_IsCustomActioned()
{
	CustomAction(bIsCustomActioned);
}
