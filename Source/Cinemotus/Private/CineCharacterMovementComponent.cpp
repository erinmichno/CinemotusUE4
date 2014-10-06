

#include "Cinemotus.h"
#include "CineCharacterMovementComponent.h"
#include "Engine.h"


UCineCharacterMovementComponent::UCineCharacterMovementComponent(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	RotationRate.Yaw = 30.0f;
	bRotationInLocalSpace = false;
	MaxSpeed = 1200.0f;

	
}



void UCineCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	// Compute new rotation
	const FQuat OldRotation = UpdatedComponent->GetComponentQuat(); //the collider
	const FQuat DeltaRotation = (RotationRate * DeltaTime).Quaternion(); //Where we can put in rotation of 
	//IF THE CHANGE IS WORLD then World*CurrentRotation*Local
	const FQuat NewRotation = bRotationInLocalSpace ? (OldRotation * DeltaRotation) : (DeltaRotation * OldRotation);

	// Compute new location
	FVector NewLocation = UpdatedComponent->GetComponentLocation();
	if (!PivotTranslation.IsZero())
	{
		const FVector OldPivot = OldRotation.RotateVector(PivotTranslation);
		const FVector NewPivot = NewRotation.RotateVector(PivotTranslation);
		NewLocation = (NewLocation + OldPivot - NewPivot);
	}

	UpdatedComponent->SetWorldLocationAndRotation(NewLocation, NewRotation);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//IF PC then PC->SetControlRotation(ViewRot)

	if (PawnOwner)
	{
		APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
		if (PC)
		{
			//PC->SetControlRotation(NewRotation.Rotator()); //if you set this then it may override the set in the controller
		}
	}
}



