// Fill out your copyright notice in the Description page of Project Settings.

// Only exists in the server.

#include "LobbyGameMode.h"
#include "GameFramework//GameStateBase.h"
#include "Blaster/GameInstance/BlasterGameInstance.h"
#include "GameFramework/GameSession.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	//if (NumberOfPlayers == 2)
	//{
	//	UWorld* World = GetWorld();
	//	if (World)
	//	{
	//		bUseSeamlessTravel = true;
	//		World->ServerTravel(FString("/Game/Maps/BlasterMap?listen")); // listen server, clients to connect to
	//	}
	//}
}

void ALobbyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
}

void ALobbyGameMode::TravelToMap(FString MapPath)
{
	UWorld* World = GetWorld();
	if (World)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel(FString::Printf(TEXT("%s?listen"), *MapPath)); // listen server, clients to connect to
	}
}

void ALobbyGameMode::KickPlayer(APlayerController* PlayerToKickController)
{
	// If player died in the lobby, kick them
	UBlasterGameInstance* BlasterGameInstance = Cast<UBlasterGameInstance>(GetGameInstance());
	if (BlasterGameInstance)
	{
		// Access (authoritative) game session and kick player
		GameSession.Get()->KickPlayer(PlayerToKickController, FText::FromString("This kick reason will not get displayed to clients."));
	}
}

