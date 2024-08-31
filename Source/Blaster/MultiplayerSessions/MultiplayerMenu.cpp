// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerMenu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"


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
		UE_LOG(LogBlasterNetworking, Error, TEXT("Couldn't set up sessions subsystem."));
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
		OnLookingForSessionValueChanged(bLookingForSessions);
		MultiplayerSessionsSubsystem->FindSessions(1000);
	}
}

void UMultiplayerMenu::CreateSession(int32 NumPublicConnections, FString DisplayName, bool bIsLAN)
{
	if (MultiplayerSessionsSubsystem && !bCreatingSession)
	{
		bCreatingSession = true;
		OnCreatingSessionValueChanged(bCreatingSession);
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, DisplayName, bIsLAN);
	}
	else
	{
		// missing log here
	}
}


void UMultiplayerMenu::OnCreateSession(bool bWasSuccessful)
{
	bCreatingSession = false;
	OnCreatingSessionValueChanged(bCreatingSession);
	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(FString::Printf(TEXT("%s?listen"), *LobbyPath));
		}
	}
	else
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("OnCreateSession returned false. Returning to main menu."));
		// todo: return to main menu
	}
}

void UMultiplayerMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	bLookingForSessions = false;
	OnLookingForSessionValueChanged(bLookingForSessions);
	if (MultiplayerSessionsSubsystem == nullptr)
	{
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
	if (!bWasSuccessful || FoundSessions.Num() == 0)
	{
		OnNoSessionsFound();
		if (!bWasSuccessful)
		{
			UE_LOG(LogBlasterNetworking, Error, TEXT("OnFindSessions wasn't successful. Returning to main menu."));
		}
	}
	else
	{
		OnSessionsFound(FoundSessions);
	}
}

void UMultiplayerMenu::JoinSession(FOnlineSessionSearchResultWrapper SessionSearchResult)
{
	if (MultiplayerSessionsSubsystem && !bJoiningSession && !bCreatingSession)
	{
		bJoiningSession = true;
		OnJoiningSessionValueChanged(bJoiningSession);
		MultiplayerSessionsSubsystem->JoinSession(SessionSearchResult.SessionSearchResult);
	}
}

void UMultiplayerMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Type::Success)
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("OnJoinSession wasn't successful. Returning to main menu."));
		// whenever we "return to main menu", I meant return to server browser, and make sure to reload server list
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
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UMultiplayerMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("OnDestroySession returned false."));
	}
}

void UMultiplayerMenu::OnStartSession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("OnStartSession returned false."));
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

