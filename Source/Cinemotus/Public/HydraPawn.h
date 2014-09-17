

#pragma once

#include "GameFramework/Pawn.h"
//#include "../../Plugins/HydraPlugin/Source/HydraPlugin/Public/HydraDelegate.h"
#include "HydraDelegate.h"
#include "HydraPawn.generated.h"

/**
 * 
 */
UCLASS()
class CINEMOTUS_API AHydraPawn : public APawn,  public HydraDelegate
{
	GENERATED_UCLASS_BODY()

		//UFUNCTION(BlueprintImplementableEvent, Category = HydraEvents)
	virtual	void HydraB1Pressed(int32 controllerNum) override;
		//Override Initialization and Tick to forward *required* hydra functions.
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
