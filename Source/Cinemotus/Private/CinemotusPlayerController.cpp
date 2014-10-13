

#include "Cinemotus.h"
#include "CinemotusPlayerController.h"
#include "IHydraPlugin.h"
#include "Engine.h"
#include "CinemotusDefaultPawn.h"
#include "CinemotusGameMode.h"
#include "Kismet/GameplayStatics.h"

ACinemotusPlayerController::ACinemotusPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
	possessedCinePawn = NULL;
	bumperTapTimer = 0;

	pawnStartingRotator = FRotator::ZeroRotator;
	 
	 currentCaptureState = ECinemotusCaptureState::ERelativeOff;
	 /*ViewPitchMin = -89.9f;
	ViewPitchMax = 89.9f;
	ViewYawMin = 0.f;
	ViewYawMax = 359.999f;
	ViewRollMin = -89.9f;
	ViewRollMax = 89.9f;
	 */
}

//




void ACinemotusPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PlayerCameraManager->ViewPitchMin = -89.9f;
	PlayerCameraManager->ViewPitchMax = 270.0f;
	PlayerCameraManager->ViewRollMin = -89.9f;
	PlayerCameraManager->ViewRollMax = 270.0f;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.5f, FColor::Yellow, PlayerCameraManager->CameraStyle.ToString());
	}

}

//Override Callable Functions - Required to forward their implementations in order to compile, cannot skip implementation as we can for events.
bool ACinemotusPlayerController::HydraIsAvailable()
{
	return HydraDelegate::HydraIsAvailable();
}
int32 ACinemotusPlayerController::HydraWhichHand(int32 controller)
{
	return HydraDelegate::HydraWhichHand(controller);
}
bool ACinemotusPlayerController::HydraGetLatestData(int32 controller, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation, FRotator& angularVelocity,
	FVector2D& joystick, int32& buttons, float& trigger, bool& docked)
{
	return HydraDelegate::HydraGetLatestData(controller, position, velocity, acceleration, rotation, angularVelocity, joystick, buttons, trigger, docked);
}
bool ACinemotusPlayerController::HydraGetHistoricalData(int32 controller, int32 historyIndex, FVector& position, FVector& velocity, FVector& acceleration, FRotator& rotation, FRotator& angularVelocity,
	FVector2D& joystick, int32& buttons, float& trigger, bool& docked)
{
	return HydraDelegate::HydraGetHistoricalData(controller, historyIndex, position, velocity, acceleration, rotation, angularVelocity, joystick, buttons, trigger, docked);
}

//Required Overrides, forward startup and tick.
void ACinemotusPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HydraTick(DeltaTime);
}


//dead zone and smoothing
float DeadZone(float val, float tolerance)
{
	if (FMath::Abs(val) < tolerance)
	{
		return 0;
	}

	return val - tolerance;


}

//
//void ACinemotusPlayerController::MoveForward(float Val)
//{
//	if (Val != 0.f)
//	{
//		if (Controller)
//		{
//			FRotator const ControlSpaceRot = Controller->GetControlRotation();
//
//			// transform to world space and add it
//			AddMovementInput(FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::X), Val);
//		}
//	}//EAxis::Y for right  and 	AddMovementInput(FVector::UpVector, Val); for UP'
//}

void ACinemotusPlayerController::HandleMovement(float DeltaTime)
{

	APawn* pawn = GetPawn();
	if (!pawn)
	{
		return;
	}

	//check velocities
	FVector velocity = HydraLatestData->controllers[0].velocity;
	FRotationMatrix mat(GetControlRotation());
	float scaleCmToMetres = 10;
	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::X), velocity.X*DeltaTime * scaleCmToMetres);
	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::Y), velocity.Y*DeltaTime * scaleCmToMetres);
	pawn->AddMovementInput(FVector::UpVector, velocity.Z*DeltaTime*scaleCmToMetres);
	//pawn->AddMovementInput(velocity, DeltaTime*10);

}

void ACinemotusPlayerController::AbsoluteTick(float DeltaTime)
{
	if (((currentCaptureState&ECinemotusCaptureState::EAbsolute) == ECinemotusCaptureState::EAbsolute) && 
		((currentCaptureState&ECinemotusCaptureState::EAbsoluteOff) == 0))
	{
		SetControlRotation(HydraLatestData->controllers[0].rotation);
		HandleMovement(DeltaTime);
	}
}
void ACinemotusPlayerController::RelativeTick(float DeltaTime)
{
	if ((currentCaptureState & ECinemotusCaptureState::ERelativeRotation) == ECinemotusCaptureState::ERelativeRotation)
	{
		UPrimitiveComponent* prim = GetPawn()->GetMovementComponent()->UpdatedComponent;
		FRotator rot = HydraLatestData->controllers[0].angular_velocity;
		const FQuat OldRotation = prim->GetComponentQuat();//GetControlRotation().Quaternion();
		const FRotator OldRotationRotator = OldRotation.Rotator();
		FRotator worldRotator = FRotator(0, DeadZone(rot.Yaw*DeltaTime, 0.0), 0);
		FRotator worldRotator1 = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.0), 0, 0);
		FRotator localRotator = FRotator(0, 0, DeadZone(rot.Roll*DeltaTime, 0.0));
		const FQuat WorldRot = worldRotator.Quaternion();
		const FQuat pitchRot = worldRotator1.Quaternion();
		const FQuat LocalRot = localRotator.Quaternion();

		////This one does roll around local forward, pitch around world right flattened and yaw around world up
		////			FQuat finalQuat = pitchRot*WorldRot*((OldRotation*LocalRot));

		FQuat finalQuat = WorldRot*((OldRotation*LocalRot)*pitchRot);

		

		/*FRotator worldRotator = FRotator(0, DeadZone(rot.Yaw*DeltaTime, 0.0) + OldRotationRotator.Yaw, 0);
		FRotator worldRotator1 = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.0) + OldRotationRotator.Pitch, 0, 0);
		FRotator localRotator = FRotator(0, 0, DeadZone(rot.Roll*DeltaTime, 0.0) + OldRotationRotator.Roll);
		const FQuat WorldRot = worldRotator.Quaternion();
		const FQuat pitchRot = worldRotator1.Quaternion();
		const FQuat LocalRot = localRotator.Quaternion();
		FQuat finalQuat = WorldRot*pitchRot*LocalRot;*/
	


		bool SetPrimDirectly = true;
		if (SetPrimDirectly)
		{
			
			if (prim)
			{
				prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), finalQuat);// not sure need
			}
		}

		SetControlRotation(finalQuat.Rotator());




		//static float currentPitch = 0.0;
		//currentPitch += 30 * DeltaTime;
		////SetControlRotation(prim->GetComponentRotation());
		//FRotator aRotator = FRotator(currentPitch, 0, 0);
		////Absolute things
		//FQuat rot = HydraLatestData->controllers[0].quat;
		////prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), aRotator);
		//FString output = FString::Printf(TEXT("in thing: "));
		//output += aRotator.ToString();
		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Red, output);
		//}
		//SetControlRotation(aRotator);

	}
	if ((currentCaptureState & ECinemotusCaptureState::ERelativeTranslation) == ECinemotusCaptureState::ERelativeTranslation)
	{
		HandleMovement(DeltaTime);
	}
}
void ACinemotusPlayerController::PlayerTick(float DeltaTime)
{
	if (bumperTapTimer > 0)
	{
		bumperTapTimer -= DeltaTime;
	}
	if (currentCaptureState & ECinemotusCaptureState::EABSOLUTE)
	{
		AbsoluteTick(DeltaTime);
	}
	else
	{
		RelativeTick(DeltaTime);
	}
	Super::PlayerTick(DeltaTime);
}
void ACinemotusPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//InputComponent->BindAction("NextPawn", IE_Pressed, this, &ACinemotusPlayerController::OnSwichPawn);
	//InputComponent->BindAction("SetDestination", IE_Released, this, &AMyProject2PlayerController::OnSetDestinationReleased);
}



void ACinemotusPlayerController::BeginPlay()
{

	Super::BeginPlay();
	HydraStartup();
	//find all spawn volume actors
	TArray<AActor*> FoundActors;


//	UGameplayStatics::GetPlayerPawn();

	//static from game engine
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);

	for (auto Actor : FoundActors)
	{
		APawn* pwn = Cast<APawn>(Actor);
		if (pwn)
		{
			PawnsInScene.Add(pwn);
		}
	}

	APawn* currentPawn = GetPawn();
	
	currentPawnIndex = 0;
	for (int i = 0; i < PawnsInScene.Num(); ++i)
	{
		if (PawnsInScene[i] == currentPawn)
		//if (PawnsInScene[i]->ActorHasTag(TEXT("CinemotusCharacter")))
		{
			currentPawnIndex = i;
			break;
		}
	}

	possessedCinePawn = Cast<ACinemotusDefaultPawn>(GetPawn());
}


void ACinemotusPlayerController::HydraB4Released(int32 controllerNum)
{
	currentCaptureState = (currentCaptureState & ECinemotusCaptureState::EABSOLUTE) ? ECinemotusCaptureState::ERelativeOff : ECinemotusCaptureState::EAbsoluteOff;

}

void ACinemotusPlayerController::HydraTriggerPressed(int32 controllerNum)
{
	currentCaptureState |= (currentCaptureState & ECinemotusCaptureState::ERELATIVE) ? ECinemotusCaptureState::ERelativeRotation : 0x0;
	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, TEXT("SSSS"));

}


 void ACinemotusPlayerController::HydraTriggerReleased(int32 controllerNum)
 {
	 currentCaptureState &= ~ECinemotusCaptureState::ERelativeRotation;
 }

 void ACinemotusPlayerController::HydraBumperPressed(int32 controllerNum)
 {
	 currentCaptureState |= (currentCaptureState & ECinemotusCaptureState::ERELATIVE) ? ECinemotusCaptureState::ERelativeTranslation : 0x0;
	 GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, TEXT("BUMPERRR"));
	 bumperTapTimer = 0.4f;
 }


 void ACinemotusPlayerController::HydraBumperReleased(int32 controllerNum)
 {
	 currentCaptureState &= ~ECinemotusCaptureState::ERelativeTranslation;

	 if (bumperTapTimer > 0.0f && (currentCaptureState & ECinemotusCaptureState::EABSOLUTE))
	 {
		 currentCaptureState = (currentCaptureState & ECinemotusCaptureState::EAbsoluteOff) == ECinemotusCaptureState::EAbsoluteOff ? ECinemotusCaptureState::EAbsolute : ECinemotusCaptureState::EAbsoluteOff;
	 }
 }


//called every frame controller undocked  
void ACinemotusPlayerController::HydraControllerMoved(int32 controller,
	FVector position, FVector velocity, FVector acceleration,
	FRotator rotation, FRotator angularVelocity)
{

}

void ACinemotusPlayerController::OnSwichPawn()
{
	//Get Current Pawn's rotation?

	if (PawnsInScene.Num() < 2)
	{
		return;
	}
	FString PowerLevelString = FString::Printf(TEXT("%d"), PawnsInScene.Num());
	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, PowerLevelString);
	currentPawnIndex = currentPawnIndex + 1 < PawnsInScene.Num() ? currentPawnIndex + 1 : 0;
	APawn* nextPawn = PawnsInScene[currentPawnIndex];
	
	Possess(nextPawn);
	SetViewTargetWithBlend(nextPawn, 0.0f);
	possessedCinePawn = Cast<ACinemotusDefaultPawn>(nextPawn);
	


}

void ACinemotusPlayerController::HydraStartReleased(int32 controllerNum)
{
	//Go to next pawn stuff
	OnSwichPawn();

}

void ACinemotusPlayerController::HydraB1Pressed(int32 controllerNum)
{


	



}



/*BAD TIMES
FRotator test = FRotator::ZeroRotator;
//if (capture) TODO: capture correctly
{
FRotator rot = HydraLatestData->controllers[0].angular_velocity;
FRotator res = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.3), DeadZone(rot.Yaw*DeltaTime, 0.3), DeadZone(rot.Roll*DeltaTime, 0.3));
//SetControlRotation(GetPawn()->GetActorRotation());
//GetPawn()->AddActorLocalRotation(res);
//

//TODO: test better!

UPrimitiveComponent* prim = GetPawn()->GetMovementComponent()->UpdatedComponent;
const FQuat OldRotation = GetControlRotation().Quaternion();//prim->GetComponentQuat(); //the collider
const FRotator OldRotationRotator = OldRotation.Rotator();


if (true)
{
FRotator worldRotator = FRotator(0, DeadZone(rot.Yaw*DeltaTime, 0.0), 0);
FRotator worldRotator1 = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.0), 0, 0);
FRotator localRotator = FRotator(0, 0, DeadZone(rot.Roll*DeltaTime, 0.0));
const FQuat WorldRot = worldRotator.Quaternion();
const FQuat pitchRot = worldRotator1.Quaternion();
const FQuat LocalRot = localRotator.Quaternion();

//This one does roll around local forward, pitch around world right flattened and yaw around world up
//			FQuat finalQuat = pitchRot*WorldRot*((OldRotation*LocalRot));

FQuat finalQuat = WorldRot*((OldRotation*LocalRot)*pitchRot);

//FQuat finalQuat = (WorldRot*OldRotation);
//	FQuat finalQuat = (WorldRot*((OldRotation*pitchRot)*LocalRot));
//	prim->SetRelativeRotation(finalQuat.Rotator());
//prim->SetWorldRotation();
prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), finalQuat);
SetControlRotation(finalQuat.Rotator());
}
else //use
{

FRotator worldRotator = FRotator(0, DeadZone(rot.Yaw*DeltaTime, 0.0) + OldRotationRotator.Yaw, 0);
FRotator worldRotator1 = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.0) + OldRotationRotator.Pitch, 0, 0);
FRotator localRotator = FRotator(0, 0, DeadZone(rot.Roll*DeltaTime, 0.0) + OldRotationRotator.Roll);
const FQuat WorldRot = worldRotator.Quaternion();
const FQuat pitchRot = worldRotator1.Quaternion();
const FQuat LocalRot = localRotator.Quaternion();
FQuat finalQuat = WorldRot*pitchRot*LocalRot;

//prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), finalQuat);


static float currentPitch = 0.0;


currentPitch += 30 * DeltaTime;

//SetControlRotation(prim->GetComponentRotation());
FRotator aRotator = FRotator(currentPitch, 0, 0);

//Absolute things
FQuat rot = HydraLatestData->controllers[0].quat;


//prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), aRotator);
FString output = FString::Printf(TEXT("in thing: "));
output += aRotator.ToString();
if (GEngine)
{
GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Red, output);
}

SetControlRotation(finalQuat.Rotator());

}







//RotationInput.Roll += DeadZone(rot.Roll*DeltaTime,0.3);
//RotationInput.Yaw += DeadZone(rot.Yaw*DeltaTime, 0.3);
//RotationInput.Pitch += DeadZone(rot.Pitch*DeltaTime, 0.3);



}
HandleMovement(DeltaTime);




*/


