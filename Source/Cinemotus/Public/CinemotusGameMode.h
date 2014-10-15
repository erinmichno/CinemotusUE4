// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"

#include "CinemotusGameMode.generated.h"

class ACameraActor;







//// New
////if (!!(Flags & EFlags::Flag1))
//
//
//// Enum to store the current state of gameplay.
//enum class EHydraJoystickState : uint8
//{
//	ESpeed,
//	EYawCrane,
//	EPlanarMovement,
//	ECamera
//};

UCLASS(minimalapi)
class ACinemotusGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()
		virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;


	UFUNCTION(BlueprintCallable, Category = Camera)
		TArray<ACameraActor*> GetCameraActors();

	UFUNCTION(BlueprintCallable, Category = Pawns)
		TArray<APawn*> GetPawnsFromBeginPlay();

	


	protected:
		

private:
	TArray<ACameraActor*> CamaeraActors;
	TArray<APawn*>  PawnsInScene;

};



