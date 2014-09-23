

#include "Cinemotus.h"
#include "CinemotusDefaultPawn.h"


ACinemotusDefaultPawn::ACinemotusDefaultPawn(const class FPostConstructInitializeProperties& PCIP)
//  : Super(PCIP)
	: Super(PCIP
	.DoNotCreateDefaultSubobject(ADefaultPawn::MeshComponentName))
{
	// Create a follow camera
	camera0 = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("camera0"));
	camera0->AttachTo(RootComponent); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	camera0->bUseControllerViewRotation = false; // Camera does not rotate relative to arm

//	bTakeCameraControlWhenPossessed = true;
	bFindCameraComponentWhenViewTarget = true;
}


