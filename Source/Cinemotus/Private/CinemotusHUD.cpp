

#include "Cinemotus.h"
#include "CinemotusHUD.h"
#include "CinemotusCharacter.h"
#include "CinemotusGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

ACinemotusHUD::ACinemotusHUD(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	//Use robotoDistanceField font as it's included in engine
	static ConstructorHelpers::FObjectFinder<UFont>HUDFontOb(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	HUDFont = HUDFontOb.Object;
}


void ACinemotusHUD::DrawHUD()
{
	FVector2D ScreenDimensions = FVector2D(Canvas->SizeX, Canvas->SizeY);

	//Call to the parent versions of DrawHUD
	Super::DrawHUD();

	ACinemotusCharacter* MyCharacter = Cast<ACinemotusCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	FString PowerLevelString = FString::Printf(TEXT("%10.1f"), FMath::Abs(MyCharacter->PowerLevel));
	DrawText(PowerLevelString, FColor::White, 50, 50, HUDFont);

	//ACinemotusGameMode * MyGameMode = Cast<ACinemotusGameMode>(UGameplayStatics::GetGameMode(this));

	//	FVector2D GameOverSize;
	//	GetTextSize(TEXT("GAME OVER!"), GameOverSize.X, GameOverSize.Y, HUDFont);
	//	DrawText(TEXT("GAME OVER!"), FColor::White, (ScreenDimensions.X - GameOverSize.X)*0.5f, (ScreenDimensions.Y - GameOverSize.Y)*0.5f);


	//HUD 50 is quite small
}
