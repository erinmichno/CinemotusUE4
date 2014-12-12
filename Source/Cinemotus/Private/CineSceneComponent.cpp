// Fill out your copyright notice in the Description page of Project Settings.

#include "Cinemotus.h"
#include "CineSceneComponent.h"


UCineSceneComponent::UCineSceneComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	LayerNumber = 1024;
	relativeScaleFactor = 1.0f;
	joystickScaleFactor = 1.0f;
}


float UCineSceneComponent::GetScaleFactor() const
{
	return relativeScaleFactor;
}
void UCineSceneComponent::SetScaleFactor(float s)
{
	relativeScaleFactor = s;
}

float UCineSceneComponent::GetJoystickScaleFactor()const
{
	return joystickScaleFactor;
}
void UCineSceneComponent::SetJoystickScaleFactor(float s)
{
	joystickScaleFactor = s;
}