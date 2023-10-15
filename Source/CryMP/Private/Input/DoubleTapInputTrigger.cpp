// CryMP, All Rights Reserved.


#include "Input/DoubleTapInputTrigger.h"
#include "EnhancedPlayerInput.h"

ETriggerState UDoubleTapInputTrigger::UpdateState_Implementation(const UEnhancedPlayerInput* PlayerInput,
                                                                 FInputActionValue ModifiedValue, float DeltaTime)
{
	if (IsActuated(ModifiedValue) && !IsActuated(LastValue))
	{
		const float CurrentTime = PlayerInput->GetOuterAPlayerController()->GetWorld()->GetRealTimeSeconds();
		if (CurrentTime - LastTappedTime < Delay)
		{
			LastTappedTime = 0;
			bIsReleased = false;
			return ETriggerState::Triggered;
		}
		LastTappedTime = CurrentTime;
	}
	
	if (!IsActuated(LastValue)) {
		bIsReleased = true;
	}
 
	if (TapAndHold) {
		if (!bIsReleased) return ETriggerState::Triggered;
	}
	
	return ETriggerState::None;
}
