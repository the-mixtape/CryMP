// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CMPCharacterMovementComponent.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


#pragma region Saved Move
UCMPCharacterMovementComponent::FSavedMove_CMP::FSavedMove_CMP()
{
	Saved_bWantsToRun = 0;
	Saved_bWantsToWalk = 0;
}

bool UCMPCharacterMovementComponent::FSavedMove_CMP::CanCombineWith(const FSavedMovePtr& NewMove,
                                                                    ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_CMP* NewCMPMove = static_cast<FSavedMove_CMP*>(NewMove.Get());

	if (NewCMPMove->Saved_bWantsToRun != Saved_bWantsToRun) return false;
	if (NewCMPMove->Saved_bWantsToWalk != Saved_bWantsToWalk) return false;

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::Clear()
{
	Super::Clear();

	Saved_bWantsToRun = 0;
	Saved_bWantsToWalk = 0;
}

uint8 UCMPCharacterMovementComponent::FSavedMove_CMP::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToRun) Result |= FLAG_Custom_0;
	if (Saved_bWantsToWalk) Result |= FLAG_Custom_1;

	return Result;
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::SetMoveFor(ACharacter* C, float InDeltaTime,
                                                                FVector const& NewAccel,
                                                                FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const auto CharacterMovement = Cast<UCMPCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToRun = CharacterMovement->Safe_bWantsToRun;
	Saved_bWantsToWalk = CharacterMovement->Safe_bWantsToWalk;
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	const auto CharacterMovement = Cast<UCMPCharacterMovementComponent>(C->GetCharacterMovement());
	CharacterMovement->Safe_bWantsToRun = Saved_bWantsToRun;
	CharacterMovement->Safe_bWantsToWalk = Saved_bWantsToWalk;
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
	NavAgentProps.bCanCrouch = true;
}

FNetworkPredictionData_Client* UCMPCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner);

	if (ClientPredictionData == nullptr)
	{
		UCMPCharacterMovementComponent* MutableThis = const_cast<UCMPCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_CMP(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UCMPCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCMPCharacterMovementComponent, CurrentGait);
}

void UCMPCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToRun = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	Safe_bWantsToWalk = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

void UCMPCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
                                                       const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	bIsAccelerating = Velocity.Size() > OldVelocity.Size();
	
	if (MovementMode == MOVE_Walking)
	{
		const auto MoveAngle =
			UKismetAnimationLibrary::CalculateDirection(Velocity, GetPawnOwner()->GetActorRotation());
		const auto AbsMoveAngle = FMath::Abs(MoveAngle);

		if (Safe_bWantsToRun && AbsMoveAngle <= Run_Angle)
		{
			SetTargetGait(EGaits::ECMS_Run, RunSettings);
			
		}
		else if (Safe_bWantsToWalk)
		{
			SetTargetGait(EGaits::ECMS_Walk, WalkSettings);
		}
		else
		{
			SetTargetGait(EGaits::ECMS_Jog, JogSettings);
		}
		
		ApplyGaitSettings();
	}
}

void UCMPCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ReplicateMovementState();
}

void UCMPCharacterMovementComponent::ReplicateMovementState()
{
	if (!GetOwner()->HasAuthority()) return;
	if (MovementMode != MOVE_Walking) return;

	if (FMath::IsNearlyEqual(MaxWalkSpeed, RunSettings.MaxWalkSpeed, 0.001f)) CurrentGait = EGaits::ECMS_Run;
	else if (FMath::IsNearlyEqual(MaxWalkSpeed, WalkSettings.MaxWalkSpeed, 0.001f)) CurrentGait = EGaits::ECMS_Walk;
	else CurrentGait = EGaits::ECMS_Jog;
}

void UCMPCharacterMovementComponent::StartRun()
{
	Safe_bWantsToWalk = false;
	Safe_bWantsToRun = true;
}

void UCMPCharacterMovementComponent::StopRun()
{
	Safe_bWantsToRun = false;
}

void UCMPCharacterMovementComponent::StartWalk()
{
	Safe_bWantsToRun = false;
	Safe_bWantsToWalk = true;
}

void UCMPCharacterMovementComponent::StopWalk()
{
	Safe_bWantsToWalk = false;
}


void UCMPCharacterMovementComponent::ToggleCrouch()
{
	bWantsToCrouch = !bWantsToCrouch;
}

void UCMPCharacterMovementComponent::StartCrouch()
{
	bWantsToCrouch = true;
}

void UCMPCharacterMovementComponent::StopCrouch()
{
	bWantsToCrouch = false;
}
#pragma endregion
