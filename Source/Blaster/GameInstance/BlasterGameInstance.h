// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Blaster/BlasterTypes/LogCategories.h"
#include "Blaster/MultiplayerSessions/MultiplayerSessionsSubsystem.h"
#include "Blaster/HUD/EmoteWheel/EmoteWheel.h"

#include "BlasterGameInstance.generated.h"


/**
 * 
 */
UCLASS(Blueprintable)
class BLASTER_API UBlasterGameInstance : public UGameInstance
{
	GENERATED_BODY()
	

public:
	UPROPERTY(Category = "Emotes", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FEmoteType> Emotes = { };

	/** Amount of dynamic platforms in the current level that do not have an ID set yet. */
	int32 NoIdDynamicPlatforms = 0;

	virtual void Init() override;

	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	FString NetworkErrorMessage = FString("");

	UMultiplayerSessionsSubsystem* GetMultiplayerSubsystem();

	void ClearChat();
	void AddChatMessage(FString Message);
private:
	UMultiplayerSessionsSubsystem* MultiplayerSubsystem = nullptr;
	TArray<FName> ChatMessages; // FName has 8 bytes so we save data


public:
	FORCEINLINE TArray<FName> GetChatMessages() { return ChatMessages; };
};
