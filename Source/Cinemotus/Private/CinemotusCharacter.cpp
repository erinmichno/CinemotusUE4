// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Cinemotus.h"
#include "CinemotusCharacter.h"
#include "CinemotusGameMode.h"
#include "Camera/CameraActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Engine.h"

//////////////////////////////////////////////////////////////////////////
// ACinemotusCharacter

ACinemotusCharacter::ACinemotusCharacter(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// Set size for collision capsule
	CapsuleComponent->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PowerLevel = 1128;

	// Configure character movement
	CharacterMovement->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	CharacterMovement->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	CharacterMovement->JumpZVelocity = 600.f;
	CharacterMovement->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraZoom_v = 300.0f;
	CameraBoom->TargetArmLength = CameraZoom_v; // The camera follows at this distance behind the character	
	CameraBoom->bUseControllerViewRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUseControllerViewRotation = false; // Camera does not rotate relative to arm


	anArrorComponent = PCIP.CreateDefaultSubobject<UArrowComponent>(this, TEXT("anArrorComponent"));
	anArrorComponent->AttachTo(RootComponent);
	//FTransform(const FQuat& InRotation, const FVector& InTranslation,
	FQuat quat(1, 0, 0, 3.14/2.0f);
	FTransform transform(quat, FVector(0, 0, 0));
	//anArrorComponent->SetWorldTransform(transform);
	//anArrorComponent->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	bFindCameraComponentWhenViewTarget = true;
	Tags.Add(TEXT("CinemotusCharacter"));
}

/////
void ACinemotusCharacter::CameraZoomIn()
{
	float a = 25.0;
	CameraZoom_v = CameraZoom_v - 25.0;

	if (CameraZoom_v <= 75.0)
	{
		CameraBoom->TargetArmLength = 75.0;
		CameraZoom_v = 75.0;
	}
	else
	{
		CameraBoom->TargetArmLength = CameraZoom_v;
	}
}

void ACinemotusCharacter::CameraZoomOut()
{
	float a = 25.0;
	CameraZoom_v = CameraZoom_v + 25.0;

	if (CameraZoom_v >= 300.0)
	{
		CameraBoom->TargetArmLength = 300.0;
		CameraZoom_v = 300.0;
	}
	else
	{
		CameraBoom->TargetArmLength = CameraZoom_v;
	}
}




void ACinemotusCharacter::HydraB1Pressed(int32 controllerNum)
{
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACinemotusCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	InputComponent->BindAction("AttachCam", IE_Pressed, this, &ACinemotusCharacter::OnSetCameraPressed);

	InputComponent->BindAxis("MoveForward", this, &ACinemotusCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ACinemotusCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ACinemotusCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ACinemotusCharacter::LookUpAtRate);

	// handle touch devices
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ACinemotusCharacter::TouchStarted);

	
}


void ACinemotusCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void ACinemotusCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACinemotusCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACinemotusCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACinemotusCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ACinemotusCharacter::HydraControllerMoved(int32 controller,
	FVector position, FVector velocity, FVector acceleration,
	FRotator rotation, FRotator angularVelocity){};


void ACinemotusCharacter::OnSetCameraPressed()
{

	//GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, TEXT("FunctionCalled"));
	ACinemotusGameMode* MyGameMode = Cast<ACinemotusGameMode>(UGameplayStatics::GetGameMode(this));

	TArray<ACameraActor*> camActors = MyGameMode->GetCameraActors();

	USceneComponent* rootCamComponent = camActors[0]->GetRootComponent();
	//rootCamComponent->GetName();
	
	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, rootCamComponent->GetName());


	rootCamComponent->AttachTo(anArrorComponent);

	//rootCamComponent->SetWorldTransform(FTransform::Identity);
	//for (int32 iCollected = 0; iCollected < CollectedActors.Num(); ++iCollected)
	//{
	//	// Cast the collected Actor to ABatteryPickup.
	//	ABatteryPickup* const TestBattery = Cast<ABatteryPickup>(CollectedActors[iCollected]);

	//	// If the cast is successful, and the battery is valid and active
	//	if (TestBattery && !TestBattery->IsPendingKill() && TestBattery->bIsActive)
	//	{
	//		// Store its battery power for adding to the character's power.
	//		BatteryPower = BatteryPower + TestBattery->PowerLevel;
	//		// Deactivate the battery
	//		TestBattery->bIsActive = false;
	//		// Call the battery's OnPickedUp function
	//		TestBattery->OnPickedUp();
	//	}
	//}
}