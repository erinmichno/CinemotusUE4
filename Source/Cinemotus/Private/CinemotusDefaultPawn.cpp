

#include "Cinemotus.h"
#include "CinemotusDefaultPawn.h"
#include "CineCharacterMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerInput.h"

//#include "HydraPlugin.h"


ACinemotusDefaultPawn::ACinemotusDefaultPawn(const class FPostConstructInitializeProperties& PCIP)
//  : Super(PCIP)
: Super((PCIP.SetDefaultSubobjectClass<UCineCharacterMovementComponent>(ADefaultPawn::MovementComponentName)).DoNotCreateDefaultSubobject(ADefaultPawn::MeshComponentName))
//: Super(PCIP.DoNotCreateDefaultSubobject(ADefaultPawn::MeshComponentName))
{
	//.DoNotCreateDefaultSubobject(ADefaultPawn::MeshComponentName)
	//Create a movement component with our movement component type
	//MovementComponent = PCIP.CreateDefaultSubobject<UCineCharacterMovementComponent>(this, ADefaultPawn::MovementComponentName);
//	MovementComponent->UpdatedComponent = CollisionComponent;

	// Create a follow camera
	camera0 = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("camera0"));
	camera0->AttachTo(RootComponent); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	//camera0->bUseControllerViewRotation = false; // Camera does not rotate relative to arm

//	bTakeCameraControlWhenPossessed = true;
	bFindCameraComponentWhenViewTarget = true;
	bAddDefaultMovementBindings = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	/*bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;*/

	camera0->bUseControllerViewRotation = true;

	//camera0->RelativeRotation;

	//RelativeRotation = (FRotationMatrix(CharacterOwner->GetActorRotation()) * FQuatRotationTranslationMatrix(OldBaseQuat, FVector::ZeroVector).GetTransposed()).Rotator();
}


void InitializeDefaultPawnInputBindings()
{
	static bool bBindingsAdded = false;
	if (!bBindingsAdded)
	{
		bBindingsAdded = true;

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::W, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::S, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::Up, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::Down, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveForward", EKeys::Gamepad_LeftY, 1.f));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight", EKeys::A, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight", EKeys::D, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveRight", EKeys::Gamepad_LeftX, 1.f));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Gamepad_LeftThumbstick, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Gamepad_RightThumbstick, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Gamepad_FaceButton_Bottom, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::LeftControl, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::SpaceBar, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::C, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::E, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_MoveUp", EKeys::Q, -1.f));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate", EKeys::Gamepad_RightX, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate", EKeys::Left, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_TurnRate", EKeys::Right, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_Turn", EKeys::MouseX, 1.f));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_LookUpRate", EKeys::Gamepad_RightY, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("DefaultPawn_LookUp", EKeys::MouseY, -1.f));
	}
}

void ACinemotusDefaultPawn::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	if (bAddDefaultMovementBindings)
	{
		InitializeDefaultPawnInputBindings();

		InputComponent->BindAxis("DefaultPawn_MoveForward", this, &ADefaultPawn::MoveForward);
		InputComponent->BindAxis("DefaultPawn_MoveRight", this, &ADefaultPawn::MoveRight);
		InputComponent->BindAxis("DefaultPawn_MoveUp", this, &ADefaultPawn::MoveUp_World);
		InputComponent->BindAxis("DefaultPawn_Turn", this, &ADefaultPawn::AddControllerYawInput);
		InputComponent->BindAxis("DefaultPawn_TurnRate", this, &ADefaultPawn::TurnAtRate);
		InputComponent->BindAxis("DefaultPawn_LookUp", this, &ADefaultPawn::AddControllerPitchInput);
		InputComponent->BindAxis("DefaultPawn_LookUpRate", this, &ADefaultPawn::LookUpAtRate);
	}

}



void ACinemotusDefaultPawn::UpdateRotationFromHydra()
{
	//could get y,p,r axes or hyrda callback or get Struct of last data 
}



