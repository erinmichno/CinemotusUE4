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

	/** the layer value of camera if order dependent. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Cinemotus)
	float LayerNumber;

	//if needed a get/ create if no found cam component / parent etc
		
protected:
	float relativeScaleFactor;

	
	
};
