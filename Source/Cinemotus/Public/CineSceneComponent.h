// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "CineSceneComponent.generated.h"

/**
 * 
 */
UCLASS()
class CINEMOTUS_API UCineSceneComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()

	float GetScaleFactor() const;
	void SetScaleFactor(float s);

	float GetJoystickScaleFactor()const;
	void SetJoystickScaleFactor(float s);

	/** the layer value of camera if order dependent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cinemotus)
	float LayerNumber;

	//if needed a get/ create if no found cam component / parent etc
		
protected:
	float relativeScaleFactor;
	float joystickScaleFactor;
	
	
};
