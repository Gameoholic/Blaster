// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameInstance.h"

void UBlasterGameInstance::Init()
{
	Super::Init();
	UEngine* Engine = GetEngine();
	if (Engine)
	{
		Engine->OnNetworkFailure().AddUObject(this, &UBlasterGameInstance::OnNetworkFailure);
		Engine->OnTravelFailure().AddUObject(this, &UBlasterGameInstance::OnTravelFailure);
	}

	// Make sure we don't have any existing session (aka hosting/connected to a session) when we start the game. This shouldn't happen but just in case.
	// If issues still happen, move this to MultiplayerMenu.cpp on initialize, but there shouldn't be problems because we destroy on network failure anyway
	MultiplayerSubsystem = MultiplayerSubsystem == nullptr ? GetSubsystem<UMultiplayerSessionsSubsystem>() : MultiplayerSubsystem;
	
	if (MultiplayerSubsystem != nullptr && MultiplayerSubsystem->SessionAlreadyExists())
	{
		UE_LOG(LogBlasterNetworking, Warning, TEXT("[BlasterGameInstance] Found existing session on GameInstance Init. Destroying."));
		MultiplayerSubsystem->DestroySession();
	}
}

void UBlasterGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	// In case the host disconnects the clients will remain in the session, so we must destroy the connection and leave it.
	// We'll return to the main menu regardless, not sure why. Probably something internal in NetworkFailure()
	UE_LOG(LogBlasterNetworking, Warning, TEXT("[BlasterGameInstance] Network Failure! %s"), *ErrorString);
	MultiplayerSubsystem = MultiplayerSubsystem == nullptr ? GetSubsystem<UMultiplayerSessionsSubsystem>() : MultiplayerSubsystem;
	if (MultiplayerSubsystem != nullptr)
	{
		UE_LOG(LogBlasterNetworking, Log, TEXT("[BlasterGameInstance] Attempting to destroy multiplayer session post network failure."));
		MultiplayerSubsystem->DestroySessionIfExists();
	}
}


void UBlasterGameInstance::OnTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	// I haven't really had any problems with server travel, but the logic here seems good so I'm keeping it.
	UE_LOG(LogBlasterNetworking, Warning, TEXT("[BlasterGameInstance] Travel Failure! %s"), *ErrorString);
	MultiplayerSubsystem = MultiplayerSubsystem == nullptr ? GetSubsystem<UMultiplayerSessionsSubsystem>() : MultiplayerSubsystem;
	if (MultiplayerSubsystem != nullptr)
	{
		UE_LOG(LogBlasterNetworking, Log, TEXT("[BlasterGameInstance] Attempting to destroy multiplayer session post travel failure."));
		MultiplayerSubsystem->DestroySessionIfExists();
	}
}
