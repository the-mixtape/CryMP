// CryMP, All Rights Reserved.


#include "Framework/CMPGameModeBase.h"
#include "Player/CMPBaseCharacter.h"
#include "Player/CMPPlayerController.h"

ACMPGameModeBase::ACMPGameModeBase()
{
	DefaultPawnClass = ACMPBaseCharacter::StaticClass();
	PlayerControllerClass = ACMPPlayerController::StaticClass();
}
