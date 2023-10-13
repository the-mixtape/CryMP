// CryMP, All Rights Reserved.


#include "Player/CMPCharacterMovementComponent.h"

#include "GameFramework/Character.h"


#pragma region Saved Move
UCMPCharacterMovementComponent::FSavedMove_CMP::FSavedMove_CMP()
{
	Saved_bWantsToSprint = 0;
}

bool UCMPCharacterMovementComponent::FSavedMove_CMP::CanCombineWith(const FSavedMovePtr& NewMove,
                                                                    ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_CMP* NewCMPMove = static_cast<FSavedMove_CMP*>(NewMove.Get());

	if (NewCMPMove->Saved_bWantsToSprint != Saved_bWantsToSprint)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::Clear()
{
	Super::Clear();

	Saved_bWantsToSprint = 0;
}

uint8 UCMPCharacterMovementComponent::FSavedMove_CMP::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToSprint) Result |= FLAG_Custom_0;

	return Result;
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::SetMoveFor(ACharacter* C, float InDeltaTime,
                                                                FVector const& NewAccel,
                                                                FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const auto CharacterMovement = Cast<UCMPCharacterMovementComponent>(C->GetCharacterMovement());
	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);
	
	const auto CharacterMovement = Cast<UCMPCharacterMovementComponent>(C->GetCharacterMovement());
	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
}
#pragma endregion

#pragma region Client Network Prediction Data
UCMPCharacterMovementComponent::FNetworkPredictionData_Client_CMP::FNetworkPredictionData_Client_CMP(
	const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
	
}

FSavedMovePtr UCMPCharacterMovementComponent::FNetworkPredictionData_Client_CMP::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_CMP());
}
#pragma endregion

#pragma region Character Movement Component 
UCMPCharacterMovementComponent::UCMPCharacterMovementComponent()
{
}

FNetworkPredictionData_Client* UCMPCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner);

	if(ClientPredictionData == nullptr)
	{
		UCMPCharacterMovementComponent* MutableThis = const_cast<UCMPCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_CMP(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	
	return ClientPredictionData;
}

void UCMPCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UCMPCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if(MovementMode == MOVE_Walking)
	{
		if(Safe_bWantsToSprint)
		{
			MaxWalkSpeed = Sprint_MaxWalkSpeed;
		}
		else
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}
}

void UCMPCharacterMovementComponent::StartSprint()
{
	Safe_bWantsToSprint = true;
}

void UCMPCharacterMovementComponent::StopSprint()
{
	Safe_bWantsToSprint = false;
}
#pragma endregion 
