// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Blaster/BlasterTypes/LogCategories.h"
#include "MultiplayerMenu.generated.h"



// Native type unavailable in BP
USTRUCT(BlueprintType)
struct FOnlineSessionSearchResultWrapper
{
	GENERATED_USTRUCT_BODY()

	FOnlineSessionSearchResult SessionSearchResult;
};

USTRUCT(BlueprintType)
struct FMultiplayerSessionInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly)
	FString SessionDisplayName;
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayerAmount;
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayerAmount;
	UPROPERTY(BlueprintReadOnly)
	int32 Ping;
	UPROPERTY(BlueprintReadOnly)
	FOnlineSessionSearchResultWrapper SessionSearchResult;


	//FMultiplayerSessionInfo();
};

const FString LobbyPath = FString(TEXT("/Game/Maps/Lobby"));

/**
 * 
 */
UCLASS()
class BLASTER_API UMultiplayerMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//void MenuSetup(int32 NumberOfPublicConnections = 10, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));
	UFUNCTION(BlueprintCallable)
	void SetupMenu();

	
	UFUNCTION(BlueprintImplementableEvent)
	void OnSessionsFound(const TArray<FMultiplayerSessionInfo>& SessionsInfo);

	UFUNCTION(BlueprintImplementableEvent)
	void OnNoSessionsFound();

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayErrorMessage(const FString& ErrorMessage);


	UPROPERTY(BlueprintReadOnly)
	bool bLookingForSessions = false;
	UPROPERTY(BlueprintReadOnly)
	bool bJoiningSession = false;
	UPROPERTY(BlueprintReadOnly)
	bool bCreatingSession = false;

protected:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UFUNCTION(BlueprintCallable)
	void LookForSessions();

	UFUNCTION(BlueprintCallable)
	void CreateSession(int32 NumPublicConnections, FString DisplayName, bool bIsLAN);

	UFUNCTION(BlueprintCallable)
	void JoinSession(FOnlineSessionSearchResultWrapper SessionSearchResult);

	void MenuTearDown();

	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;



};
