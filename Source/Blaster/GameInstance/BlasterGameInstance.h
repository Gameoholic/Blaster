// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Blaster/BlasterTypes/LogCategories.h"
#include "Blaster/MultiplayerSessions/MultiplayerSessionsSubsystem.h"

#include "BlasterGameInstance.generated.h"


/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterGameInstance : public UGameInstance
{
	GENERATED_BODY()
	

public:
	/** Amount of dynamic platforms in the current level that do not have an ID set yet. */
	int32 NoIdDynamicPlatforms = 0;

	virtual void Init() override;

	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void OnTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

private:
	UMultiplayerSessionsSubsystem* MultiplayerSubsystem = nullptr;

};
