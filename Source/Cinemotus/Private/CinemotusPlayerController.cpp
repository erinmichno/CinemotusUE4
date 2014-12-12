

#include "Cinemotus.h"
#include "CinemotusPlayerController.h"
#include "IHydraPlugin.h"
#include "Engine.h"
#include "CinemotusDefaultPawn.h"
#include "CinemotusGameMode.h"
#include "CineSceneComponent.h"
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
	 currentJoystickState = ECinemotusJoystickState::EYawCrane;
	 fSpeedMulitplier = 1.0f;
	 fJoystickMultiplier = 1.0f;
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

	retVal = FString::Printf(TEXT("Speed Multiplier: %10.4f and JoystickMultiplier %10.4f"), fSpeedMulitplier, fJoystickMultiplier);
	return retVal;
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
	//joystickHeaderText = ECinemotusJoystickState::ToString(currentJoystickState);
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
	float scalar = 2.5;
	if (useHydraMotion)
	{
		FRotationMatrix cMat(HydraLatestData->controllers[CAM_HAND].rotation);
		velRel.X = FVector::DotProduct(cMat.GetScaledAxis(EAxis::X), velocity);
		velRel.Y = FVector::DotProduct(cMat.GetScaledAxis(EAxis::Y), velocity);
		velRel.Z = FVector::DotProduct(cMat.GetScaledAxis(EAxis::Z), velocity); //take motion and make relative to the orientation of the controller
	}
	//velocity.X*DeltaTime * scaleCmToMetres*fSpeedMulitplier +

	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::X), velRel.X*DeltaTime * scalar*fSpeedMulitplier + vXYandCrane.X);
	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::Y), velRel.Y*DeltaTime * scalar*fSpeedMulitplier + vXYandCrane.Y);
	pawn->AddMovementInput(mat.GetScaledAxis(EAxis::Z), velRel.Z*DeltaTime * scalar*fSpeedMulitplier);
	pawn->AddMovementInput(FVector::UpVector, vXYandCrane.Z);


	//Add Movement input for offhand

	FVector xPlanar = mat.GetScaledAxis(EAxis::X);
	xPlanar.Z = 0;
	bool didNorm = xPlanar.Normalize();
	if (!didNorm)
	{ 
		xPlanar.X = 1.0; xPlanar.Normalize(); }
	pawn->AddMovementInput(xPlanar, offHandPlanarMovement.X);


	FVector yPlanar = mat.GetScaledAxis(EAxis::Y);
	yPlanar.Z = 0;
	didNorm = yPlanar.Normalize();
	if (!didNorm) { yPlanar.Y = 1.0; yPlanar.Normalize(); }
	pawn->AddMovementInput(yPlanar, offHandPlanarMovement.Y);



}


void ACinemotusPlayerController::AbsoluteTick(float DeltaTime)
{

	
	TotalYawAbs += addYaw;
	UPrimitiveComponent* prim = GetPawn()->GetMovementComponent()->UpdatedComponent;
	//USceneComponent* sComponent = GetPawn()->GetRootComponent();
	//sComponent->SetRelativeLocation;

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


void ACinemotusPlayerController::HandleOffHandJoysticks(FVector2D joyPos)
{
	offHandPlanarMovement = FVector::ZeroVector;
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	offHandPlanarMovement.X += joyPos.Y*DeltaTime*fJoystickMultiplier*50.0f;
	offHandPlanarMovement.Y += joyPos.X*DeltaTime*fJoystickMultiplier*50.0f;
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
		vXYandCrane.X += joyPos.Y*DeltaTime*fJoystickMultiplier*50.0f;
		vXYandCrane.Y += joyPos.X*DeltaTime*fJoystickMultiplier*50.0f;
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
			vXYandCrane.Z += joyPos.Y*DeltaTime*fJoystickMultiplier*50.0f;
		}
	}
		//
		break;


	}
}


void ACinemotusPlayerController::UpdateSpeedMultiplier(bool increment, float & refSpeedMultiplier)
{
	float multip = increment ? 2 : 0.5f;
	refSpeedMultiplier *= multip;
	//clamp
	refSpeedMultiplier = FMath::Clamp(refSpeedMultiplier, 0.00625f, 16.0f);
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

	
	
	HandleJoysticks(HydraLatestData->controllers[CAM_HAND].joystick);
	HandleOffHandJoysticks(HydraLatestData->controllers[CAM_HAND == 0 ? 1 : 0].joystick);
	
	
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

	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Cine_CreateCam", EKeysHydra::HydraRightJoystickClick));
	UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Cine_CreateCam", EKeysHydra::HydraLeftJoystickClick));
	InputComponent->BindAction("Cine_CreateCam", IE_Pressed, this, &ACinemotusPlayerController::AddCinePawn);
}

void ACinemotusPlayerController::AddCinePawn()
{
//	pawnToAdd->
	APawn* pawnToAdd = NULL;

	int newIndex = 0;
	while (newIndex < PawnsInScene.Num() &&!PawnsInScene[newIndex]->bHidden)
	{
		newIndex++;
	}//keep going till we find a  hidden one

	if (newIndex >= PawnsInScene.Num())
	{
		//add a dude
		UWorld* const World = GetWorld();
		if (World)
		{
			// Set the spawn parameters
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = Instigator;

			// Get a random location to spawn at
			FVector SpawnLocation = GetPawn()->GetActorLocation();
			// Get a random rotation for the spawned item
			FRotator SpawnRotation = GetPawn()->GetActorRotation();
			// spawn the pickup
			pawnToAdd = GetWorld()->SpawnActor<ACinemotusDefaultPawn>(ACinemotusDefaultPawn::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
		}
		
	}
	else
	{
		pawnToAdd = PawnsInScene[newIndex];
		APawn* pwn = GetPawn();
		pawnToAdd->SetActorLocationAndRotation(pwn->GetActorLocation(), pwn->GetActorRotation()); //make this one added to where we are right now

	}


	

	pawnToAdd->SetActorTickEnabled(true);
	pawnToAdd->SetActorEnableCollision(true);
	pawnToAdd->SetActorHiddenInGame(false);
	
	SwitchToPawn(pawnToAdd);

	
}


int32 ACinemotusPlayerController::GetPawnListIndex(APawn* p)
{
	int32 retVal = -1;
	for (int i = 0; i < PawnsInScene.Num(); ++i)
	{
		if (PawnsInScene[i] == p)
		{
			retVal = i;
			break;
		}
	}
	return retVal;
}

void ACinemotusPlayerController::SortPawnInSceneList()
{
	PawnsInScene.Sort(
		[](const APawn & a, const APawn  & b)
	{
		const UCineSceneComponent* cineDataA = a.FindComponentByClass<UCineSceneComponent>();

		if (cineDataA != NULL)
		{
			const UCineSceneComponent* cineDataB = b.FindComponentByClass<UCineSceneComponent>();
			if (cineDataB != NULL)
			{
				return cineDataA->LayerNumber < cineDataB->LayerNumber;
			}
			return true;
		}
		return false;
	}

	);
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
		ACinemotusDefaultPawn* pwn = Cast<ACinemotusDefaultPawn>(Actor);
		if (pwn)
		{
			PawnsInScene.Add(pwn);
			UCineSceneComponent* cineDataA = pwn->FindComponentByClass<UCineSceneComponent>();
			cineDataA->LayerNumber = rand(); //just give random for now

		}
	}
	
	PawnsInScene.Sort(
		[](const APawn & a, const APawn  & b)
	{
		const UCineSceneComponent* cineDataA = a.FindComponentByClass<UCineSceneComponent>();

		if (cineDataA != NULL)
		{
			const UCineSceneComponent* cineDataB = b.FindComponentByClass<UCineSceneComponent>();
			if (cineDataB != NULL)
			{
				return cineDataA->LayerNumber < cineDataB->LayerNumber;
			}
			return true;
		}
		return false;
	}
		
		);

	APawn* currentPawn = GetPawn();
	
	currentPawnIndex = 0;


	for (int i = 0; i < PawnsInScene.Num(); ++i)
	{
		if (PawnsInScene[i] == currentPawn)
		//if (PawnsInScene[i]->ActorHasTag(TEXT("CinemotusCharacter")))
		{
			currentPawnIndex = i -1;
			if (currentPawnIndex < 0)
			{
				currentPawnIndex = PawnsInScene.Num() - 1;
			}
			break;
		}
	}

	possessedCinePawn = Cast<ACinemotusDefaultPawn>(GetPawn());

	OnSwichPawn();
}

void ACinemotusPlayerController::HydraB1Released(int32 controllerNum)//translation
{
	if (controllerNum != CAM_HAND)
	{
		currentCaptureState = (currentCaptureState & ECinemotusCaptureState::EABSOLUTE) ? ECinemotusCaptureState::ERelativeOff : ECinemotusCaptureState::EAbsoluteOff;
	}
	else
	{
		OnSwichPawn(false);
	}
	//HandleJoystickStateChange(ECinemotusJoystickState::EPlanarMovement);
}
void ACinemotusPlayerController::HydraB2Released(int32 controllerNum) //speed
{
	if (controllerNum != CAM_HAND)
	{
		UpdateSpeedMultiplier(false, fJoystickMultiplier);
		
		//set 
	}
	else
	{
		UpdateSpeedMultiplier(false, fSpeedMulitplier);
	}
	SetCineData(GetPawn());
	//HandleJoystickStateChange(ECinemotusJoystickState::ESpeed);
}
void ACinemotusPlayerController::HydraB3Released(int32 controllerNum)//yaw crane
{
	
	if (controllerNum != CAM_HAND)
	{
		currentCaptureState = (currentCaptureState & ECinemotusCaptureState::EABSOLUTE) ? ECinemotusCaptureState::ERelativeOff : ECinemotusCaptureState::EAbsoluteOff;
	}
	else
	{
		OnSwichPawn(true);
	}
	//HandleJoystickStateChange(ECinemotusJoystickState::EYawCrane);
}

void ACinemotusPlayerController::HydraJoystickReleased(int32 controllerNum)
{

}


void ACinemotusPlayerController::HydraB4Released(int32 controllerNum)
{
	if (controllerNum != CAM_HAND)
	{
		UpdateSpeedMultiplier(true, fJoystickMultiplier);
	}
	else
	{
		UpdateSpeedMultiplier(true, fSpeedMulitplier);
	}
	SetCineData(GetPawn());

	//currentCaptureState = (currentCaptureState & ECinemotusCaptureState::EABSOLUTE) ? ECinemotusCaptureState::ERelativeOff : ECinemotusCaptureState::EAbsoluteOff;

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

void ACinemotusPlayerController::OnSwichPawn(bool increase)
{
	//Get Current Pawn's rotation?

	if (PawnsInScene.Num() < 1)
	{
		return;
	}
	FString numstr = FString::Printf(TEXT("%d"), PawnsInScene.Num());
	GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, numstr);
	int startIndex = currentPawnIndex;
	do
	{
		if (increase)
		{
			currentPawnIndex = currentPawnIndex + 1 < PawnsInScene.Num() ? currentPawnIndex + 1 : 0;
		}
		else
		{
			currentPawnIndex = currentPawnIndex - 1 < 0 ? PawnsInScene.Num() - 1 : currentPawnIndex - 1;
		}
	} while (PawnsInScene[currentPawnIndex]->bHidden && currentPawnIndex != startIndex); //keep going till we find a non hidden one
	APawn* nextPawn = PawnsInScene[currentPawnIndex];
	
	Possess(nextPawn);
	SetViewTargetWithBlend(nextPawn, 0.0f);
	possessedCinePawn = Cast<ACinemotusDefaultPawn>(nextPawn);
	//GET JOYSTICK DATA
	GetCineData(GetPawn());

}

void ACinemotusPlayerController::SwitchToPawn(APawn* p)
{

	int pawnIndex = GetPawnListIndex(p);
	if (pawnIndex == -1)//not in list
	{
		//add
		PawnsInScene.Add(p);

	}

	SortPawnInSceneList();//incase layer change
	Possess(p);
	SetViewTargetWithBlend(p, 0.0f);
	possessedCinePawn = Cast<ACinemotusDefaultPawn>(p);
	//GET JOYSTICK DATA
	GetCineData(GetPawn());
}

void ACinemotusPlayerController::SetCineData(AActor* currentActor)
{
	UCineSceneComponent* cineData = currentActor->FindComponentByClass<UCineSceneComponent>();
	if (cineData != NULL)
	{
		cineData->SetScaleFactor(fSpeedMulitplier);
		cineData->SetJoystickScaleFactor(fJoystickMultiplier);
	}
	else
	{
		//todo: add cine component
		//UCineSceneComponent* cineData = 
		/*

		void AYourActor::CreateComponent(UClass* CompClass,const FVector& Location, const FRotator& Rotation, const FName& AttachSocket=NAME_None)
		{
		FName YourObjectName("Hiiii");

		//CompClass can be a BP
		UPrimitiveComponent* NewComp = ConstructObject<UPrimitiveComponent>( CompClass, this, YourObjectName);
		if(!NewComp)
		{
		return NULL;
		}
		//~~~~~~~~~~~~~

		NewComp->RegisterComponent();        //You must ConstructObject with a valid Outer that has world, see above
		NewComp->SetWorldLocation(Location);
		NewComp->SetWorldRotation(Rotation);
		NewComp->AttachTo(GetRootComponent(),SocketName,EAttachLocation::KeepWorldPosition);
		//could use different than Root Comp
		}
		*/
	}
}

void ACinemotusPlayerController::GetCineData(AActor* currentActor)
{
	UCineSceneComponent* cineData = currentActor->FindComponentByClass<UCineSceneComponent>();
	if (cineData != NULL)
	{
		//Get the data
		fSpeedMulitplier = cineData->GetScaleFactor();
		fJoystickMultiplier = cineData->GetJoystickScaleFactor();
		joystickHeaderText = cineData->GetOwner()->GetName();
	}
}

void ACinemotusPlayerController::HydraStartReleased(int32 controllerNum)
{
	//Go to next pawn stuff
	OnSwichPawn();

}










