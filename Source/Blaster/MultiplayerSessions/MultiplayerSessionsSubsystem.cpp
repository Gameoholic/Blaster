// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"


UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))

{

}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString SessionDisplayName, bool bIsLAN)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Attempting to create session."));
	if (!IsValidSessionInterface())
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't create session because session interface isn't valid."));
		MultiplayerOnCreateSessionComplete.Broadcast(false);
		return;
	}

	if (SessionAlreadyExists())
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't create session because one already exists! Attempting to destroy it now."));
		MultiplayerOnCreateSessionComplete.Broadcast(false);
		DestroySession();
		return;
	}
	DestroySession(); // temp delete

	// Store the delegate in a FDelegateHandle so we can later remove it from the delegate list
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = bIsLAN; // IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("BlasterGame"), FString("BlasterGame"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing); // Diffrentiate between this and other steam games
	LastSessionSettings->Set(FName("SessionDisplayName"), SessionDisplayName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing); // Diffrentiate between this and other steam games
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->Set(SEARCH_KEYWORDS, FString("BlasterGame"), EOnlineDataAdvertisementType::ViaOnlineService);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't create session."));

		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		// Broadcast our own custom delegate
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Trying to look for sessions."));

	if (!IsValidSessionInterface())
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't look for sessions because session interface isn't valid."));
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	if (SessionAlreadyExists())
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't look for sessions because we are already in session! Attempting to destroy it now."));
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		DestroySession();
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; // this might cause issues
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	LastSessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, FString("BlasterGame"), EOnlineComparisonOp::Equals);


	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Error while looking for sessions!"));
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Attempting to join session."));
	if (!IsValidSessionInterface())
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't join session because session interface isn't valid."));
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	if (SessionAlreadyExists())
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't join session because we are already in session! Attempting to destroy it now."));
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		DestroySession();
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Error while joining session."));
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

bool UMultiplayerSessionsSubsystem::SessionAlreadyExists()
{
	if (!IsValidSessionInterface())
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't check if session already exists because session interface isn't valid."));
		return true;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	return ExistingSession != nullptr;
}

void UMultiplayerSessionsSubsystem::DestroySessionIfExists()
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Destroying session only if exists."));
	if (SessionAlreadyExists())
	{
		UE_LOG(LogBlasterNetworking, Warning, TEXT("[MultiplayerSessionsSubsystem] Session exists, destroying."));
		DestroySession();
	}
	else
	{
		UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Session doesn't exist."));
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Attempting to destroy session."));
	if (!IsValidSessionInterface())
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Session interface invalid while destroying session."));
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		UE_LOG(LogBlasterNetworking, Error, TEXT("[MultiplayerSessionsSubsystem] Couldn't destroy session."));
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

bool UMultiplayerSessionsSubsystem::IsValidSessionInterface()
{
	if (!SessionInterface)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			SessionInterface = Subsystem->GetSessionInterface();
		}
	}
	return SessionInterface.IsValid();
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Created session complete with result: %b"), bWasSuccessful);

	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Find sessions complete with result: %b"), bWasSuccessful);

	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (bWasSuccessful && LastSessionSearch->SearchResults.Num() <= 0)
	{
		UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Found 0 sessions."));
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), true);
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Join session complete with result: %d"), Result);

	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogBlasterNetworking, Log, TEXT("[MultiplayerSessionsSubsystem] Destroy session complete with result: %b"), bWasSuccessful);

	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

