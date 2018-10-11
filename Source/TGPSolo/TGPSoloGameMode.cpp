// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TGPSoloGameMode.h"
#include "TGPSoloHUD.h"
#include "TGPSoloCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATGPSoloGameMode::ATGPSoloGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ATGPSoloHUD::StaticClass();
}
