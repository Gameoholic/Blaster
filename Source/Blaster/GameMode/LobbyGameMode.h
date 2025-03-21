// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Tick(float DeltaTime) override;

	void TravelToMap(FString MapPath);
	virtual void KickPlayer(class APlayerController* PlayerToKickController);

};
