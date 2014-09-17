// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "CinemotusGameMode.generated.h"

UCLASS(minimalapi)
class ACinemotusGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()
		virtual void Tick(float DeltaSeconds) override;

};



