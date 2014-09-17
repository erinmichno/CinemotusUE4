// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Cinemotus.h"
#include "CinemotusGameMode.h"
#include "CinemotusCharacter.h"
#include "CinemotusHUD.h"

ACinemotusGameMode::ACinemotusGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/MyCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	//static ConstructorHelpers::FClassFinder<APlayerController> PlayerBPClass(TEXT("/Game/Blueprints/BP_HydraPlayerController"));
	//BP_HydraPlayerController
	//APickup* const SpawnedPickup = World->SpawnActor<APickup>(WhatToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	//PlayerControllerClass = PlayerBPClass.Class;


	//static ConstructorHelpers::FObjectFinder<UClass> PlayerPawnBPClass(TEXT("Class'/Game/Blueprints/MyCharacter.MyCharacter_C'"));
	//if (PlayerPawnBPClass.Object != NULL)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Object;

	//}

	////set the default HUD Class
	HUDClass = ACinemotusHUD::StaticClass();
}


void ACinemotusGameMode::Tick(float DeltaSeconds) 
{

	ACinemotusCharacter* MyCharacter = Cast<ACinemotusCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}
