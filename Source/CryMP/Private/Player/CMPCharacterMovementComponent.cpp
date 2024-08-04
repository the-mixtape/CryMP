// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CMPCharacterMovementComponent.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


#pragma region Saved Move
UCMPCharacterMovementComponent::FSavedMove_CMP::FSavedMove_CMP()
{
	Saved_bWantsToJog = 0;
}

bool UCMPCharacterMovementComponent::FSavedMove_CMP::CanCombineWith(const FSavedMovePtr& NewMove,
                                                                    ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_CMP* NewCMPMove = static_cast<FSavedMove_CMP*>(NewMove.Get());

	if (NewCMPMove->Saved_bWantsToJog != Saved_bWantsToJog) return false;

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::Clear()
{
	Super::Clear();

	Saved_bWantsToJog = 0;
}

uint8 UCMPCharacterMovementComponent::FSavedMove_CMP::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToJog) Result |= FLAG_Custom_0;

	return Result;
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::SetMoveFor(ACharacter* C, float InDeltaTime,
                                                                FVector const& NewAccel,
                                                                FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const auto CharacterMovement = Cast<UCMPCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToJog = CharacterMovement->Safe_bWantsToJog;
}

void UCMPCharacterMovementComponent::FSavedMove_CMP::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	const auto CharacterMovement = Cast<UCMPCharacterMovementComponent>(C->GetCharacterMovement());
	CharacterMovement->Safe_bWantsToJog = Saved_bWantsToJog;
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

void UCMPCharacterMovementComponent::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration)
	{
		// Preserve our replicated acceleration
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
	}
	else
	{
		Super::SimulateMovement(DeltaTime);
	}
}

void UCMPCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	CurrentGait = EGaits::ECMS_Walk;
	UseGaitSettings(WalkSettings);
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

	DOREPLIFETIME_CONDITION(UCMPCharacterMovementComponent, Safe_bWantsToJog, COND_SkipOwner);
}

void UCMPCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToJog = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UCMPCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
                                                       const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (MovementMode == MOVE_Walking)
	{
		const auto MoveAngle =
			UKismetAnimationLibrary::CalculateDirection(Velocity, GetPawnOwner()->GetActorRotation());
		const auto AbsMoveAngle = FMath::Abs(MoveAngle);

		if (Safe_bWantsToJog && AbsMoveAngle <= Jog_Angle)
		{
			if (CurrentGait != EGaits::ECMS_Jog)
			{
				CurrentGait = EGaits::ECMS_Jog;
				UseGaitSettings(JogSettings);
			}
		}
		else
		{
			if (CurrentGait != EGaits::ECMS_Walk)
			{
				CurrentGait = EGaits::ECMS_Walk;
				UseGaitSettings(WalkSettings);
			}
		}
	}
}

void UCMPCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCMPCharacterMovementComponent::StartJog()
{
	Safe_bWantsToJog = true;
	ServerSetWantToJog(true);
}

void UCMPCharacterMovementComponent::StopJog()
{
	Safe_bWantsToJog = false;
	ServerSetWantToJog(false);
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

void UCMPCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
}

void UCMPCharacterMovementComponent::SetWantToJog(bool Value)
{
	if (!GetOwner()->HasAuthority())
	{
		ServerSetWantToJog(Value);
	}

	Safe_bWantsToJog = Value;
}

void UCMPCharacterMovementComponent::ServerSetWantToJog_Implementation(bool Value)
{
	SetWantToJog(Value);
}
#pragma endregion
