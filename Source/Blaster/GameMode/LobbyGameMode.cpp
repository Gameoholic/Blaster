// Fill out your copyright notice in the Description page of Project Settings.

// Only exists in the server.

#include "LobbyGameMode.h"
#include "GameFramework//GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	if (NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Maps/BlasterMap?listen")); // listen server, clients to connect to
		}
	}
}

void ALobbyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
}
