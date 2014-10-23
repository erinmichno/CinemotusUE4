

#include "Cinemotus.h"
#include "CinemotusPlayerController.h"
#include "IHydraPlugin.h"
#include "Engine.h"
#include "CinemotusDefaultPawn.h"
#include "CinemotusGameMode.h"
//#include "HyrdaSingleController.h"
#include "Kismet/GameplayStatics.h"

static const int CAM_HAND = 1;

ACinemotusPlayerController::ACinemotusPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
	possessedCinePawn = NULL;
	bumperTapTimer = 0;

	pawnStartingRotator = FRotator::ZeroRotator;
	bHydraVerboseHUD = true;
	 
	 currentCaptureState = ECinemotusCaptureState::ERelativeOff;
	 currentJoystickState = ECinemotusJoystickState::ESpeed;
	 fSpeedMulitplier = 1.0f;
	 joystickHeaderText = ECinemotusJoystickState::ToString(currentJoystickState);
	 joystickVerboseText = BuildVerboseJoystickText(currentJoystickState);
	 /*ViewPitchMin = -89.9f;
	ViewPitchMax = 89.9f;
	ViewYawMin = 0.f;
	ViewYawMax = 359.999f;
	ViewRollMin = -89.9f;
	ViewRollMax = 89.9f;
	 */
	 TotalYawAbs = addYaw = 0;
	 vXYandCrane = FVector::ZeroVector;
}

//

FString ACinemotusPlayerController::BuildVerboseJoystickText(const uint8 state) const
{
	FString retVal = TEXT("UNKNOWN");
	switch (state)
	{
	case ECinemotusJoystickState::ESpeed:
	{
		
		retVal = FString::Printf(TEXT("Current Speed Multiplier: %10.4f"), fSpeedMulitplier);
	}
		break;
	case ECinemotusJoystickState::EYawCrane:
	{
		retVal = FString::Printf(TEXT("YAW CRANE DETAILS"));
	}
		break;
	case ECinemotusJoystickState::EPlanarMovement:
	{
		retVal = FString::Printf(TEXT("MOVEMENT DETAILS"));
	}
		break;
	default:
		retVal = TEXT("Unhandled State");

	}
	return retVal;
}



void ACinemotusPlayerController::HandleJoystickStateChange(uint8 requestedState)
{
	if (requestedState != currentJoystickState)
	{
		currentJoystickState = requestedState;
		joystickHeaderText = ECinemotusJoystickState::ToString(currentJoystickState);
	}
}




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
HydraControllerHand ACinemotusPlayerController::HydraWhichHand(int32 controllerId)	//call to determine which hand the controller is held in. Determined and reset on controller docking.
{
	return HydraDelegate::HydraWhichHand(controllerId);
}
sixenseControllerDataUE* ACinemotusPlayerController::HydraGetLatestData(int32 controllerId)
{
	return HydraDelegate::HydraGetLatestData(controllerId);
}
 sixenseControllerDataUE* ACinemotusPlayerController::HydraGetHistoricalData(int32 controllerId, int32 historyIndex)
 {
	 return HydraDelegate::HydraGetHistoricalData( controllerId,  historyIndex);
 }

//Required Overrides, forward startup and tick.
void ACinemotusPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HydraTick(DeltaTime);
	joystickHeaderText = ECinemotusJoystickState::ToString(currentJoystickState);
	joystickVerboseText = BuildVerboseJoystickText(currentJoystickState);
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


void ACinemotusPlayerController::HandleMovementAbs(float DeltaTime, bool useHydraMotion = false)
{
	APawn* pawn = GetPawn();
	if (!pawn)
	{
		return;
	}

	

	//check velocities
	FVector velocity = useHydraMotion ? HydraLatestData->controllers[CAM_HAND].velocity : FVector::ZeroVector;
	FVector velRel = FVector(velocity);
	FRotationMatrix mat(GetControlRotation());
	float scaleCmToMetres = 10;
	if (useHydraMotion)
	{
		FRotationMatrix cMat(HydraLatestData->controllers[CAM_HAND].rotation);
		velRel.X = FVector::DotProduct(cMat.GetScaledAxis(EAxis::X), velocity);
		velRel.Y = FVector::DotProduct(cMat.GetScaledAxis(EAxis::Y), velocity);
		velRel.Z = FVector::DotProduct(cMat.GetScaledAxis(EAxis::Z), velocity); //take motion and make relative to the orientation of the controller




	}
	//velocity.X*DeltaTime * scaleCmToMetres*fSpeedMulitplier +

	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::X), velRel.X*DeltaTime * scaleCmToMetres*fSpeedMulitplier + vXYandCrane.X);
	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::Y), velRel.Y*DeltaTime * scaleCmToMetres*fSpeedMulitplier + vXYandCrane.Y);
	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::Z), velRel.Z*DeltaTime * scaleCmToMetres*fSpeedMulitplier);
	pawn->AddMovementInput(FVector::UpVector, vXYandCrane.Z);

}

//void ACinemotusPlayerController::HandleMovement(float DeltaTime, bool useHydraMotion= false)
//{
//
//	APawn* pawn = GetPawn();
//	if (!pawn)
//	{
//		return;
//	}
//
//	//check velocities
//	FVector velocity = useHydraMotion ? HydraLatestData->controllers[CAM_HAND].velocity : FVector::ZeroVector;
//	FRotationMatrix mat(GetControlRotation());
//	float scaleCmToMetres = 10;
//	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::X), velocity.X*DeltaTime * scaleCmToMetres*fSpeedMulitplier + vXYandCrane.X);
//	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::Y), velocity.Y*DeltaTime * scaleCmToMetres*fSpeedMulitplier + vXYandCrane.Y);
//	pawn->AddMovementInput(FVector::UpVector, velocity.Z*DeltaTime*scaleCmToMetres*fSpeedMulitplier + vXYandCrane.Z);
//}

void ACinemotusPlayerController::AbsoluteTick(float DeltaTime)
{

	
	TotalYawAbs += addYaw;
	UPrimitiveComponent* prim = GetPawn()->GetMovementComponent()->UpdatedComponent;
	bool SetPrimDirectly = true;
	FQuat finalQuat;

	if (((currentCaptureState&ECinemotusCaptureState::EAbsolute) == ECinemotusCaptureState::EAbsolute) && 
		((currentCaptureState&ECinemotusCaptureState::EAbsoluteOff) == 0))
	{
		finalQuat = FRotator(0, TotalYawAbs, 0).Quaternion()*(HydraLatestData->controllers[CAM_HAND].quat);
	}
	else
	{
		finalQuat =  FRotator(0, addYaw, 0).Quaternion()*prim->GetComponentQuat();
	}
	SetControlRotation(finalQuat.Rotator());
	if (SetPrimDirectly && prim)
	{
		prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), finalQuat);// not sure need
	}


	HandleMovementAbs(DeltaTime, ((currentCaptureState&ECinemotusCaptureState::EAbsolute) == ECinemotusCaptureState::EAbsolute));
}
void ACinemotusPlayerController::RelativeTick(float DeltaTime)
{

	UPrimitiveComponent* prim = GetPawn()->GetMovementComponent()->UpdatedComponent;
	bool SetPrimDirectly = true;
	FQuat finalQuat;
	if ((currentCaptureState & ECinemotusCaptureState::ERelativeRotation) == ECinemotusCaptureState::ERelativeRotation)
	{
		FRotator rot = HydraLatestData->controllers[CAM_HAND].angular_velocity;
		const FQuat OldRotation = prim->GetComponentQuat();//GetControlRotation().Quaternion(); //TODO: hold onto a quaternion potentially
		const FRotator OldRotationRotator = OldRotation.Rotator();
		FRotator worldRotator = FRotator(0, DeadZone(rot.Yaw*DeltaTime, 0.0) + addYaw, 0);
		FRotator worldRotator1 = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.0), 0, 0);
		FRotator localRotator = FRotator(0, 0, DeadZone(rot.Roll*DeltaTime, 0.0));
		const FQuat WorldRot = worldRotator.Quaternion();
		const FQuat pitchRot = worldRotator1.Quaternion();
		const FQuat LocalRot = localRotator.Quaternion();

		////This one does roll around local forward, pitch around world right flattened and yaw around world up
		////			FQuat finalQuat = pitchRot*WorldRot*((OldRotation*LocalRot));

		finalQuat = WorldRot*((OldRotation*LocalRot)*pitchRot);
	}
	else
	{
		finalQuat = FRotator(0, addYaw, 0).Quaternion()*prim->GetComponentQuat();
	}

	SetControlRotation(finalQuat.Rotator());
	if (SetPrimDirectly && prim)
	{
		prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), finalQuat);// not sure need
	}



	HandleMovementAbs(DeltaTime, (currentCaptureState & ECinemotusCaptureState::ERelativeTranslation) == ECinemotusCaptureState::ERelativeTranslation);

}



void ACinemotusPlayerController::HandleJoysticks(FVector2D joyPos)
{

	addYaw = 0;
	vXYandCrane = FVector::ZeroVector;
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	switch (currentJoystickState)
	{
	case ECinemotusJoystickState::ESpeed:
		UpdateSpeedMultiplier(joyPos.Y);
		break;
	case ECinemotusJoystickState::EPlanarMovement:
		//
		vXYandCrane.X += joyPos.Y*DeltaTime*fSpeedMulitplier*200.0f;
		vXYandCrane.Y += joyPos.X*DeltaTime*fSpeedMulitplier*200.0f;
		break;
	case ECinemotusJoystickState::EYawCrane:
	{
		if (FMath::Abs(joyPos.X) >= FMath::Abs(joyPos.Y))
		{
			//add Yaw amount
			addYaw += joyPos.X*DeltaTime*60.0f;
		}
		else
		{
			vXYandCrane.Z += joyPos.Y*DeltaTime*fSpeedMulitplier*200.0f;
		}
	}
		//
		break;


	}
}





void ACinemotusPlayerController::UpdateSpeedMultiplier(float val)
{
	//if (Input.GetKey(KeyCode.KeypadPlus) || Input.GetKey(KeyCode.Equals))
	//{
	//	float z = 1 + 0.1f * Time.deltaTime * 4;
	//	targetZoomLevel *= z;
	//}
	//else if (Input.GetKey(KeyCode.KeypadMinus) || Input.GetKey(KeyCode.Minus))
	//{
	//	float z = 1 + 0.1f * Time.deltaTime * 4;
	//	targetZoomLevel *= 1 / z;
	//}
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (FMath::Abs(val) > 1.1)
	{
		return;
	}

	if (val > 0.1f)
	{
		float z = 1.0f + val*DeltaTime * 4;
		fSpeedMulitplier *= z;
	}
	else if (val < -0.1f)
	{
		float z = 1.0f + -val*DeltaTime*4;
		fSpeedMulitplier *= 1 / z;
	}
	//clamp
	fSpeedMulitplier = FMath::Clamp(fSpeedMulitplier, 0.00625f, 16.0f);

}


void ACinemotusPlayerController::PlayerTick(float DeltaTime)
{
	if (bumperTapTimer > 0)
	{
		bumperTapTimer -= DeltaTime;
	}

	//GET JOYSTICK DATA
	FVector2D joysticks = HydraLatestData->controllers[CAM_HAND].joystick;
	HandleJoysticks(HydraLatestData->controllers[CAM_HAND].joystick);
	
	

	//HANDLE MOTION CONTROL
	if (currentCaptureState & ECinemotusCaptureState::EABSOLUTE)
	{
		AbsoluteTick(DeltaTime);
	}
	else
	{
		RelativeTick(DeltaTime);
	}

	//COMBINE MOTION AND JOYSTICK FOR FINAL DATA

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

void ACinemotusPlayerController::HydraB1Released(int32 controllerNum)//translation
{
	HandleJoystickStateChange(ECinemotusJoystickState::EPlanarMovement);
}
void ACinemotusPlayerController::HydraB2Released(int32 controllerNum) //speed
{

	HandleJoystickStateChange(ECinemotusJoystickState::ESpeed);
}
void ACinemotusPlayerController::HydraB3Released(int32 controllerNum)//yaw crane
{

	HandleJoystickStateChange(ECinemotusJoystickState::EYawCrane);
}

void ACinemotusPlayerController::HydraJoystickReleased(int32 controllerNum)
{

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








