// Fill out your copyright notice in the Description page of Project Settings.


#include "Guns/GunPartParent.h"
#include "Kismet/KismetStringLibrary.h"


void AGunPartParent::FindTopSocket(FName& OutSocketName, UMeshComponent*& OutMeshComponent) const
{
	OutMeshComponent = GetComponentByClass<UMeshComponent>();
	const auto SocketNames = OutMeshComponent->GetAllSocketNames();

	OutSocketName = "None";
	for (const auto SocketName : SocketNames)
	{
		if (UKismetStringLibrary::StartsWith(SocketName.ToString(), TopPrefix))
		{
			OutSocketName = SocketName;
			return;
		}
	}
}

void AGunPartParent::GetAttachedInternalLoop(const AAssemblableParent* Part, TArray<AGunPartParent*>& Parts) const
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
