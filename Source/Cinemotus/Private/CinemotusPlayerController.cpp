

#include "Cinemotus.h"
#include "CinemotusPlayerController.h"
#include "IHydraPlugin.h"
#include "Engine.h"

ACinemotusPlayerController::ACinemotusPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	PrimaryActorTick.bCanEverTick = true;
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


void ACinemotusPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
}
void ACinemotusPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("NextPawn", IE_Pressed, this, &ACinemotusPlayerController::OnSwichPawn);
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
}


void ACinemotusPlayerController::OnSwichPawn()
{
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
	


}

void ACinemotusPlayerController::HydraB1Pressed(int32 controllerNum)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, TEXT("B1 PRESSED"));
	}
}


