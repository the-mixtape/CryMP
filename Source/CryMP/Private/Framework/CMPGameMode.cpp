// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CMPGameMode.h"
#include "Player/CMPCharacter.h"
#include "Player/CMPPlayerController.h"

ACMPGameMode::ACMPGameMode()
{
	DefaultPawnClass = ACMPCharacter::StaticClass();
	PlayerControllerClass = ACMPPlayerController::StaticClass();
}
