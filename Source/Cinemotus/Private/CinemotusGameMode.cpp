// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Cinemotus.h"
#include "CinemotusGameMode.h"
#include "CinemotusCharacter.h"
#include "CinemotusHUD.h"
#include "CinemotusPlayerController.h"
#include "MySpectatorPawn.h"
#include "Camera/CameraActor.h"



TArray<ACameraActor*> ACinemotusGameMode::GetCameraActors()
{ return CamaeraActors; }

TArray<APawn*> ACinemotusGameMode::GetPawnsFromBeginPlay()
{
	return PawnsInScene;
}

ACinemotusGameMode::ACinemotusGameMode(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/MyCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;//AMySpectatorPawn::StaticClass();///PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = ACinemotusPlayerController::StaticClass();
	//static ConstructorHelpers::FClassFinder<APlayerController> PlayerBPClass(TEXT("/Game/Blueprints/BP_HydraPlayerController"));
	//BP_HydraPlayerController
	//APickup* const SpawnedPickup = World->SpawnActor<APickup>(WhatToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
	//PlayerControllerClass = PlayerBPClass.Class;
	//this->PlayerControllerClass

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


void ACinemotusGameMode::BeginPlay()
{
	Super::BeginPlay();  //when overriding fcn tend to want to call parent behaviour most times

	//find all spawn volume actors
	TArray<AActor*> FoundActors;

	//static from game engine
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraActor::StaticClass(), FoundActors);

	for (auto Actor : FoundActors)
	{
		APawn* pwn = Cast<APawn>(Actor);
		if (pwn)
		{
			PawnsInScene.Add(pwn);
		}
		ACameraActor* SpawnVolumeActor = Cast<ACameraActor>(Actor);
		if (SpawnVolumeActor)
		{
			CamaeraActors.Add(SpawnVolumeActor);
		}
	}


	//SetCurrentState(ETutorialCodePlayState::EPlaying);


}