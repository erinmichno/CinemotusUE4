

#include "Cinemotus.h"
#include "HydraPawn.h"
#include "IHydraPlugin.h"
#include "Engine.h"


AHydraPawn::AHydraPawn(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{

}


//Required Overrides
void AHydraPawn::BeginPlay()
{
	Super::BeginPlay();

	//Actors by default aren't attached to the input chain, so we enable input for this actor to forward Key and Gamepad Events
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	EnableInput(PC);

	//Required Hydra Initialization
	HydraStartup();
}

void AHydraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Required Hydra Tick
	HydraTick(DeltaTime);
}


void AHydraPawn::HydraB1Pressed(int32 controllerNum) 
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Yellow, TEXT("B1 PRESSED") );
	}
}
