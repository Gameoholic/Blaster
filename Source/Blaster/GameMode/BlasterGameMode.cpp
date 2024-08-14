// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Platform/DynamicPlatform.h"


// none of the parameters are null
void ABlasterGameMode::PlayerKilled(ABlasterCharacter* KilledPlayer, ABlasterPlayerController* KilledPlayerController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* KilledPlayerState = KilledPlayerController ? Cast<ABlasterPlayerState>(KilledPlayerController->PlayerState) : nullptr;

	if (KilledPlayer->IsKilled())
	{
		return;
	}
	if (AttackerPlayerState && AttackerPlayerState != KilledPlayerState)
	{
		AttackerPlayerState->ServerAddToScore(1.0f);
		AttackerPlayerState->ServerAddToKills(1);
	}
	if (KilledPlayerState)
	{
		KilledPlayerState->ServerAddToDeaths(1);
	}
	if (KilledPlayer)
	{
		KilledPlayer->ServerKill();
	}
}

// none of the parameters are null
void ABlasterGameMode::PlayerKilled(ABlasterCharacter* KilledPlayer, ABlasterPlayerController* KilledPlayerController)
{
	ABlasterPlayerState* KilledPlayerState = KilledPlayerController ? Cast<ABlasterPlayerState>(KilledPlayerController->PlayerState) : nullptr;
	if (KilledPlayer->IsKilled())
	{
		return;
	}

	if (KilledPlayerState)
	{
		KilledPlayerState->ServerAddToDeaths(1);
	}
	if (KilledPlayer)
	{
		KilledPlayer->ServerKill();
	}
}

// params may be null
void ABlasterGameMode::RespawnPlayer(ACharacter* KilledPlayer, AController* KilledPlayerController)
{
	if (KilledPlayer)
	{
		KilledPlayer->Reset();
		KilledPlayer->Destroy();
	}
	if (KilledPlayerController)
	{
		// Select random player start to respawn at
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 SelectedPlayerStartIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(KilledPlayerController, PlayerStarts[SelectedPlayerStartIndex]);
	}
}

void ABlasterGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
}
