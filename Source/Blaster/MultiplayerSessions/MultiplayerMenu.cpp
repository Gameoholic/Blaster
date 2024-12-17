// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerMenu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Blaster/GameInstance/BlasterGameInstance.h"


void UMultiplayerMenu::SetupMenu()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();

		// Display network error message if there is any from Game Instance
		UBlasterGameInstance* CastedGameInstance = Cast<UBlasterGameInstance>(GameInstance);
		if (CastedGameInstance && CastedGameInstance->NetworkErrorMessage.Len() > 0)
		{
			DisplayErrorMessage(CastedGameInstance->NetworkErrorMessage);
			CastedGameInstance->NetworkErrorMessage = FString(""); // Reset error message
		}
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
	else
	{
		// UI Error Message not necessary here
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerMenu] Couldn't set up sessions subsystem."));
	}
}

bool UMultiplayerMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

void UMultiplayerMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}


void UMultiplayerMenu::LookForSessions()
{
	if (MultiplayerSessionsSubsystem && !bLookingForSessions && !bCreatingSession && !bJoiningSession)
	{
		bLookingForSessions = true;
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
	else
	{
		// No need to display error message
		UE_LOG(LogBlasterNetworking, Warning, TEXT("[MultiplayerMenu] Cannot look for sessions, this is probably fine."));
	}
}

void UMultiplayerMenu::CreateSession(int32 NumPublicConnections, FString DisplayName, bool bIsLAN)
{
	if (MultiplayerSessionsSubsystem && !bCreatingSession)
	{
		bCreatingSession = true;
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, DisplayName, bIsLAN);
	}
	else
	{
		// No need to display error message
		UE_LOG(LogBlasterNetworking, Warning, TEXT("[MultiplayerMenu] Cannot create session, this is probably fine."));
	}
}


void UMultiplayerMenu::OnCreateSession(bool bWasSuccessful)
{
	bCreatingSession = false;
	if (bWasSuccessful)
	{
		UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerMenu] Create session was successful. Travelling to it."));
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString::Printf(TEXT("%s?listen"), *LobbyPath));
		}
	}
	else
	{
		DisplayErrorMessage("Couldn't create server. Please try again.");
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerMenu] Create session was not successful."));
	}
}

void UMultiplayerMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerMenu] Found %d total sessions, checking them now."), SessionResults.Num());

	bLookingForSessions = false;
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		DisplayErrorMessage("Couldn't look for servers. Please try again.");
		UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerMenu] Find sessions error - subsystem is null."));
		return;
	}

	if (!bWasSuccessful)
	{
		DisplayErrorMessage("Couldn't look for servers. Please try again.");
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerMenu] Session search not successful due to an error."));
		return;
	}

	TArray<FMultiplayerSessionInfo> FoundSessions;
	for (auto SessionResult : SessionResults)
	{
		FString BlasterGameValue = ""; // Diffrentiate between this and other steam games
		SessionResult.Session.SessionSettings.Get(FName("BlasterGame"), BlasterGameValue);
		FString SessionDisplayName = "";
		SessionResult.Session.SessionSettings.Get(FName("SessionDisplayName"), SessionDisplayName);

		if (BlasterGameValue == "BlasterGame") // Check that the session actually belongs to our game
		{
			FMultiplayerSessionInfo SessionInfo;
			SessionInfo.SessionDisplayName = SessionDisplayName;
			SessionInfo.CurrentPlayerAmount = SessionResult.Session.NumOpenPrivateConnections + 1;
			SessionInfo.MaxPlayerAmount = SessionResult.Session.NumOpenPublicConnections + 1;
			SessionInfo.Ping = SessionResult.PingInMs;
			FOnlineSessionSearchResultWrapper SearchResultWrapper;
			SearchResultWrapper.SessionSearchResult = SessionResult;
			SessionInfo.SessionSearchResult = SearchResultWrapper;

			FoundSessions.Add(SessionInfo);
		}
	}
	if (FoundSessions.Num() == 0)
	{
		UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerMenu] Found 0 compatible sessions."));
		OnNoSessionsFound();
	}
	else
	{
		UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerMenu] Found %d compatible sessions."), FoundSessions.Num());
		OnSessionsFound(FoundSessions);
	}
}

void UMultiplayerMenu::JoinSession(FOnlineSessionSearchResultWrapper SessionSearchResult)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerMenu] Attempting to join session."));
	if (MultiplayerSessionsSubsystem && !bJoiningSession && !bCreatingSession)
	{
		bJoiningSession = true;
		MultiplayerSessionsSubsystem->JoinSession(SessionSearchResult.SessionSearchResult);
	}
	else
	{
		UE_LOG(LogBlasterNetworking, Warning, TEXT("[MultiplayerMenu] Cannot join session, this is probably fine."));
		DisplayErrorMessage("Couldn't join the server. Please try again.");
	}
}

void UMultiplayerMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerMenu] OnJoinSession called with result: %d"), Result);

	if (Result != EOnJoinSessionCompleteResult::Type::Success)
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerMenu] OnJoinSession wasn't successful."));
		DisplayErrorMessage("Couldn't join the server. Please try again.");
		return;
	}
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerMenu] Travelling to game."));
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UMultiplayerMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerMenu] Couldn't destroy session."));
		DisplayErrorMessage("An error occurred.");
	}
}

void UMultiplayerMenu::OnStartSession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerMenu] Couldn't start session."));
		DisplayErrorMessage("An error occurred.");
	}
}

void UMultiplayerMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}



