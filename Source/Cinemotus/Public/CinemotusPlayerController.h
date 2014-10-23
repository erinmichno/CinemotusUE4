

#pragma once

#include "GameFramework/PlayerController.h"
#include "HydraDelegate.h"
#include "Engine.h"
#include "CinemotusPlayerController.generated.h"


class ACinemotusDefaultPawn;
class HydraSingleController;
/**
 * 
 */


namespace ECinemotusJoystickState
{
	static const uint8 EYawCrane = 0x01;
	static const uint8 EPlanarMovement = 0x02;
	static const uint8 ESpeed = 0x04;
}

namespace ECinemotusJoystickState
{
	inline const FString ToString(const uint8 State)
	{

		FString returnVal = TEXT("Joystick Mode: ");
		switch (State)
		{
		case EYawCrane:
			returnVal += TEXT("Yaw and Crane");
			break;
		case EPlanarMovement:
			returnVal += TEXT("Planar Movement");
			break;
		case ESpeed:
			returnVal += TEXT("Speed");
			break;
		default:
			returnVal += TEXT("UNKNOWN");
		}
		return returnVal;
	}

}

namespace ECinemotusCaptureState
{
	
	static const uint8 EAbsolute = 0x01;
	static const uint8 ERelativeRotation = 0x02;
	static const uint8 ERelativeTranslation = 0x04;
	static const uint8 ERelativeOff = 0x08;
	static const uint8 EAbsoluteOff = 0x10;
	static const uint8 EABSOLUTE = EAbsolute | EAbsoluteOff;
	static const uint8 ERELATIVE = ERelativeRotation | ERelativeTranslation | ERelativeOff;
	
}

namespace ECinemotusCaptureState
{
	inline const FString ToString(const uint8 State)
	{
		const FString ON = TEXT("ON");
		const FString OFF = TEXT("OFF");

		FString returnVal = TEXT("Absolute Mode: tap bumper to turn ");
		if ((State & EABSOLUTE) > 0)
		{
			returnVal += (State & EAbsoluteOff) == EAbsoluteOff ? ON : OFF;
		}
		else //relative
		{
			returnVal = TEXT("Relative Mode: Rotation , ");
			returnVal += ((State&ERelativeRotation) == ERelativeRotation ? ON : OFF);
			returnVal += TEXT(" Translation ");
			returnVal += ((State&ERelativeTranslation) == ERelativeTranslation ? ON : OFF);
			//returnVal = FString::Printf(TEXT("Relative Mode: Rotation %s, Translation %s"), ((State&ERelativeRotation) == ERelativeRotation ? ON : OFF), ((State&ERelativeTranslation) == ERelativeTranslation ? ON : OFF));
		}
		return returnVal;
	}
}

//ENUM_CLASS_FLAGS(ECinemotusCaptureState);


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
	//Callable Blueprint functions - Need to be defined for direct access
	virtual bool HydraIsAvailable() override;
	virtual HydraControllerHand HydraWhichHand(int32 controllerId) override;	//call to determine which hand the controller is held in. Determined and reset on controller docking.
	virtual sixenseControllerDataUE* HydraGetLatestData(int32 controllerId) override;
	virtual sixenseControllerDataUE* HydraGetHistoricalData(int32 controllerId, int32 historyIndex) override;

	UFUNCTION(BlueprintCallable, Category = HydraFunctions)
		uint8 GetCurrentCaptureState() const;

	UFUNCTION(BlueprintCallable, Category = HydraFunctions)
		 bool ShouldDrawHydraHUDVerbose() const;

	UFUNCTION(BlueprintCallable, Category = HydraFunctions)
	FString GetJoystickVerboseText() const;

	UFUNCTION(BlueprintCallable, Category = HydraFunctions)
	FString GetJoystickHeaderText() const;

	//Override Initialization and Tick to forward *required* hydra functions.

	virtual void Tick(float DeltaTime) override;


	virtual void HydraStartReleased(int32 controllerNum)override;
	virtual void HydraB1Pressed(int32 controllerNum) override;

	virtual void HydraTriggerPressed(int32 controllerNum) override;
	virtual void HydraTriggerReleased(int32 controllerNum) override;

	virtual void HydraBumperPressed(int32 controllerNum) override;
	virtual void HydraBumperReleased(int32 controllerNum) override;

	virtual void HydraJoystickReleased(int32 controllerNum) override;
	virtual void HydraB1Released(int32 controllerNum)override;
	virtual void HydraB2Released(int32 controllerNum)override;
	virtual void HydraB3Released(int32 controllerNum)override;

	virtual void HydraB4Released(int32 controllerNum)override;
	virtual void PostInitializeComponents()override;

	//UFUNCTION(BlueprintImplementableEvent, Category = HydraEvents)
		virtual void HydraControllerMoved(int32 controller,
		FVector position, FVector velocity, FVector acceleration,
		FRotator rotation, FRotator angularVelocity) override;

		//virtual void HydraJoystickMoved(int32 controller, FVector2D movement)override;

protected:
	//FQuat YawWorld, PitchWorld, RollLocal;
	//FRotator RollPitchYawRotator;
	//void HandleMovement(float DeltaTime, bool useHydraMotion);
	void HandleMovementAbs(float DeltaTime, bool useHydraMotion);

	
	void HandleNewCaptureState(uint8 newState);
	
	void AbsoluteTick(float DeltaTime);
	void RelativeTick(float DeltaTime);

	uint8 currentCaptureState;
	uint8 currentJoystickState;
	FString joystickHeaderText, joystickVerboseText;

	void HandleJoystickStateChange(uint8 requestedState);
	FString BuildVerboseJoystickText(const uint8 state) const;
	void HandleJoysticks(FVector2D joyPos);


	float bumperTapTimer;

	bool bHydraVerboseHUD;
	float fSpeedMulitplier;
	void UpdateSpeedMultiplier(float val);

	
private:
	TArray<APawn*> PawnsInScene;
	int32 currentPawnIndex;

	ACinemotusDefaultPawn* possessedCinePawn;

	FRotator pawnStartingRotator;

	float addYaw, TotalYawAbs;
	FVector vXYandCrane;

	
	

	
	
};

FORCEINLINE uint8 ACinemotusPlayerController::GetCurrentCaptureState() const
{
	return currentCaptureState;
}


FORCEINLINE bool ACinemotusPlayerController::ShouldDrawHydraHUDVerbose() const
{
	return bHydraVerboseHUD;
}

FORCEINLINE FString ACinemotusPlayerController::GetJoystickVerboseText() const
{
	return joystickVerboseText;
}

FORCEINLINE FString ACinemotusPlayerController::GetJoystickHeaderText() const
{
	return joystickHeaderText;
}

