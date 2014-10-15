

#pragma once

#include "GameFramework/DefaultPawn.h"
#include "CinemotusDefaultPawn.generated.h"

/**
 * 
 */
UCLASS()
class CINEMOTUS_API ACinemotusDefaultPawn : public ADefaultPawn
{
	GENERATED_UCLASS_BODY()

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	TSubobjectPtr<class UCameraComponent> camera0;

	//make a dolly struct Speed / Yaw  / Crane / planar movement
	//make a cam struct Orientation / position
protected:

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	//void Initi

	void UpdateRotationFromHydra();
	
};
