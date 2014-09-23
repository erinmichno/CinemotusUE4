

#pragma once

#include "GameFramework/PlayerController.h"
#include "CinemotusPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CINEMOTUS_API ACinemotusPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()
	
	virtual void BeginPlay() override;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = Pawn)
	void OnSwichPawn();
private:
	TArray<APawn*> PawnsInScene;
	int32 currentPawnIndex;
	

	
	
};
