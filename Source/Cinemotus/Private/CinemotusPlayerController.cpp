

#include "Cinemotus.h"
#include "CinemotusPlayerController.h"
#include "IHydraPlugin.h"
#include "Engine.h"
#include "CinemotusDefaultPawn.h"

ACinemotusPlayerController::ACinemotusPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
	possessedCinePawn = NULL;


	pawnStartingRotator = FRotator::ZeroRotator;
	 controllerStartingRotator = FRotator::ZeroRotator;
	 capture = false;
	 TransCapture = false;
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
	if (!TransCapture)
	{
		return;
	}
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
void ACinemotusPlayerController::PlayerTick(float DeltaTime)
{
	FRotator test = FRotator::ZeroRotator;
	if (capture)
	{
		FRotator rot = HydraLatestData->controllers[0].angular_velocity;
		FRotator res = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.3), DeadZone(rot.Yaw*DeltaTime, 0.3), DeadZone(rot.Roll*DeltaTime, 0.3));
		//SetControlRotation(GetPawn()->GetActorRotation());
		//GetPawn()->AddActorLocalRotation(res);
		//
		//GetPawn()->SetActorRelativeRotation(rot - controllerStartingRotator);
		//SetControlRotation(   rot );//RotationInput
		//RotationInput.Roll += (rot.Roll - controllerStartingRotator.Roll);
		//RotationInput.Yaw += (rot.Yaw - controllerStartingRotator.Yaw);
		//float yaw  = rot.Y
		//TODO: test better!

		UPrimitiveComponent* prim = GetPawn()->GetMovementComponent()->UpdatedComponent;
		const FQuat OldRotation = prim->GetComponentQuat(); //the collider
		const FRotator OldRotationRotator = OldRotation.Rotator();


		if (true)
		{
			FRotator worldRotator = FRotator(0, DeadZone(rot.Yaw*DeltaTime, 0.3), 0);
			FRotator worldRotator1 = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.3), 0, 0);
			FRotator localRotator = FRotator(0, 0, DeadZone(rot.Roll*DeltaTime, 0.3));
			const FQuat WorldRot = worldRotator.Quaternion();
			const FQuat pitchRot = worldRotator1.Quaternion();
			const FQuat LocalRot = localRotator.Quaternion();

			//This one does roll around local forward, pitch around local right and yaw around world up
			//	FQuat finalQuat = WorldRot*((OldRotation*LocalRot)*pitchRot);


			FQuat finalQuat = (WorldRot*((OldRotation*pitchRot)*LocalRot));
			prim->SetRelativeRotation(finalQuat.Rotator());
			//prim->SetWorldRotation();
			//prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), finalQuat);
			SetControlRotation(finalQuat.Rotator());
		}
		else
		{

			FRotator worldRotator = FRotator(0, DeadZone(rot.Yaw*DeltaTime, 0.3) + OldRotationRotator.Yaw, 0);
			FRotator worldRotator1 = FRotator(DeadZone(rot.Pitch*DeltaTime, 0.3) + OldRotationRotator.Pitch, 0, 0);
			FRotator localRotator = FRotator(0, 0, DeadZone(rot.Roll*DeltaTime, 0.3) + OldRotationRotator.Roll);
			const FQuat WorldRot = worldRotator.Quaternion();
			const FQuat pitchRot = worldRotator1.Quaternion();
			const FQuat LocalRot = localRotator.Quaternion();
			FQuat finalQuat = WorldRot*pitchRot*LocalRot;

			prim->SetWorldLocationAndRotation(prim->GetComponentLocation(), finalQuat);
			SetControlRotation(prim->GetComponentRotation());
		}


	



		
		//RotationInput.Roll += DeadZone(rot.Roll*DeltaTime,0.3);
		//RotationInput.Yaw += DeadZone(rot.Yaw*DeltaTime, 0.3);
		//RotationInput.Pitch += DeadZone(rot.Pitch*DeltaTime, 0.3);


		
	}
	HandleMovement(DeltaTime);


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

 void ACinemotusPlayerController::HydraTriggerPressed(int32 controllerNum) 
{
	 if (controllerNum == 0)//todo which hands etc etc
	 {
		 capture = true;
		 pawnStartingRotator = GetControlRotation();//GetPawn()->GetActorRotation(); //need to get relative Rotation etc from components later
		 controllerStartingRotator = HydraLatestData->controllers[0].rotation;
		
		 
		 GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, TEXT("SSSS"));
	 }
}


 void ACinemotusPlayerController::HydraTriggerReleased(int32 controllerNum)
 {
	 if (controllerNum == 0)//todo which hands etc etc
	 {
		 capture = false;
	 }
 }

 void ACinemotusPlayerController::HydraBumperPressed(int32 controllerNum)
 {
	 if (controllerNum == 0)//todo which hands etc etc
	 {
		 TransCapture = true;
		 

		 GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, TEXT("BUMPERRR"));
	 }
 }


 void ACinemotusPlayerController::HydraBumperReleased(int32 controllerNum)
 {
	 if (controllerNum == 0)//todo which hands etc etc
	 {
		 TransCapture = false;
	 }
 }


//called every frame controller undocked  
void ACinemotusPlayerController::HydraControllerMoved(int32 controller,
	FVector position, FVector velocity, FVector acceleration,
	FRotator rotation, FRotator angularVelocity)
{

//AddActorLocalRotation deltaRotation(rotation for this frame!)
	//rotation.ToString();
	static FRotator lastFrameRotator = FRotator::ZeroRotator;

	FRotator rot = HydraLatestData->controllers[0].rotation;
	if (capture)
	{
		//GetPaw\\\\\n()->AddActorLocalRotation(rotation);
		//GetPawn()->SetActorRelativeRotation(rot - controllerStartingRotator);
		//SetControlRotation(   rot );//RotationInput
		
		controllerStartingRotator = rot;
	}


	

//	FString output = FString::Printf(TEXT("in thing: "));
//	output +=  rotation.ToString();
	//GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, output);

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


	FRotator rotation = HydraLatestData->controllers[controllerNum].rotation;

	FRotator initialRotation = FRotator(10, 20, 30);

	FRotator currentRotOFController = FRotator(355, 15, 10);

	FRotator whatShouldBe = FRotator(345, -5, -20);

	FQuat initQuat = initialRotation.Quaternion();
	FQuat currentQuatController = currentRotOFController.Quaternion();
	FQuat difference = currentQuatController*initQuat.Inverse();
	FQuat diff2 = initQuat.Inverse() * currentQuatController;//closest to 345, -5, -20
	FQuat diff3 = initQuat * currentQuatController.Inverse();
	FRotator two = diff2.Rotator();
	FRotator three = diff3.Rotator();

	FRotator rest = difference.Rotator();
	FString testString = rest.ToString();



	//FRotator deltaROtation 


	FString output = FString::Printf(TEXT("in thing: "));
	output += rotation.ToString();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Green,testString );
	}
}


