

#pragma once

#include "GameFramework/Pawn.h"
#include "CinemotusCameraPawn.generated.h"

/**
 * 
 */
UCLASS()
class CINEMOTUS_API ACinemotusCameraPawn : public APawn
{
	GENERATED_UCLASS_BODY()

		/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	TSubobjectPtr<class UCameraComponent> PawnCamera;
	
};
