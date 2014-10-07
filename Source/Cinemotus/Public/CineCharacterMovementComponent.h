

#pragma once

#include "GameFramework/FloatingPawnMovement.h"
#include "CineCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class CINEMOTUS_API UCineCharacterMovementComponent : public UFloatingPawnMovement
{
	GENERATED_UCLASS_BODY()

		/** How fast to update roll/pitch/yaw of UpdatedComponent */
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RotatingComponent)
		FRotator RotationRate;

	/** Translation of pivot point. Always relative to current rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RotatingComponent)
		FVector PivotTranslation;

	/** Whether rotation is applied in local or world space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RotatingComponent)
		uint32 bRotationInLocalSpace : 1;

	//Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//End UActorComponent Interface

	
	

	

protected:




	
	
};
