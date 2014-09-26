

#include "Cinemotus.h"
#include "CinemotusCameraPawn.h"


ACinemotusCameraPawn::ACinemotusCameraPawn(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// Create a follow camera
	PawnCamera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("PawnCamera"));
	PawnCamera->AttachTo(RootComponent); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	PawnCamera->bUseControllerViewRotation = false; // Camera does not rotate relative to arm



}


