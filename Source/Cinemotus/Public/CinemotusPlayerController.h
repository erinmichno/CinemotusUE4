

#pragma once

#include "GameFramework/PlayerController.h"
#include "HydraDelegate.h"
#include "CinemotusPlayerController.generated.h"


class ACinemotusDefaultPawn;
/**
 * 
 */
UCLASS()
class CINEMOTUS_API ACinemotusPlayerController : public APlayerController, public HydraDelegate
{
	GENERATED_UCLASS_BODY()
	
	virtual void BeginPlay() override;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = Pawn)
	void OnSwichPawn();


	//FROM HydraDelegate
	/** Check if the hydra is available/plugged in.*/
	UFUNCTION(BlueprintCallable, Category = HydraFunctions)
		bool HydraIsAvailable();
	/** Detemine which hand you're holding the hydra in. Determined and reset on docking the controller.*/
	UFUNCTION(BlueprintCallable, Category = HydraFunctions)
		int32 HydraWhichHand(int32 controller);
	/** Poll for latest data. Returns false if data is unavailable.*/
	UFUNCTION(BlueprintCallable, Category = HydraFunctions)
		bool HydraGetLatestData(int32 controller, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation, FRotator& angularVelocity,
		FVector2D& joystick, int32& buttons, float& trigger, bool& docked);
	/** Poll for historical data. Valid index is 0-9. Returns false if data is unavailable.*/
	UFUNCTION(BlueprintCallable, Category = HydraFunctions)
		bool HydraGetHistoricalData(int32 controller, int32 historyIndex, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation, FRotator& angularVelocity,
		FVector2D& joystick, int32& buttons, float& trigger, bool& docked);

	//Override Initialization and Tick to forward *required* hydra functions.

	virtual void Tick(float DeltaTime) override;


	virtual void HydraStartReleased(int32 controllerNum)override;
	virtual void HydraB1Pressed(int32 controllerNum) override;

	virtual void HydraTriggerPressed(int32 controllerNum) override;

	virtual void HydraTriggerReleased(int32 controllerNum) override;


	virtual void HydraBumperPressed(int32 controllerNum) override;

	virtual void HydraBumperReleased(int32 controllerNum) override;

	virtual void PostInitializeComponents()override;

	//UFUNCTION(BlueprintImplementableEvent, Category = HydraEvents)
		virtual void HydraControllerMoved(int32 controller,
		FVector position, FVector velocity, FVector acceleration,
		FRotator rotation, FRotator angularVelocity) override;

protected:
	//FQuat YawWorld, PitchWorld, RollLocal;
	//FRotator RollPitchYawRotator;
	void HandleMovement(float DeltaTime);
private:
	TArray<APawn*> PawnsInScene;
	int32 currentPawnIndex;

	ACinemotusDefaultPawn* possessedCinePawn;

	FRotator pawnStartingRotator;
	FRotator controllerStartingRotator;
	bool capture, TransCapture;
	

	
	
};
