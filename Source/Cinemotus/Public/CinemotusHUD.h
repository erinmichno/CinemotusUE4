

#pragma once

#include "GameFramework/HUD.h"
#include "CinemotusHUD.generated.h"

/**
 * 
 */
UCLASS()
class CINEMOTUS_API ACinemotusHUD : public AHUD
{
	GENERATED_UCLASS_BODY()
		UPROPERTY()
		UFont* HUDFont;

	virtual void DrawHUD() override;
	
	
};
