// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Platform/DynamicPlatform.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "Blaster/GameState/BlasterGameState.h"

//namespace MatchState
//{
//	const FName Cooldown = FName("Cooldown");
//}

ABlasterGameMode::ABlasterGameMode()
{
	//bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	//LevelStartingTime = GetWorld()->GetTimeSeconds();
}

//void ABlasterGameMode::OnMatchStateSet()
//{
//	Super::OnMatchStateSet();
//
//	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
//	{
//		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
//		if (BlasterPlayer)
//		{
//			BlasterPlayer->OnMatchStateSet(MatchState);
//		}
//	}
//}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (MatchState == MatchState::WaitingToStart)
	//{
	//	CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	//	if (CountdownTime <= 0.0f)
	//	{
	//		StartMatch();
	//	}
	//}
	//else if (MatchState == MatchState::InProgress)
	//{
	//	CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	//	if (CountdownTime <= 0.0f)
	//	{
	//		SetMatchState(MatchState::Cooldown);
	//	}
	//}



	// UPDATE SESSION SETTINGS MID-GAME:
	// if (!sessionInterface) do all of the following: also store both in variables, do similarly to subsystem.cpp
	//test:
		//IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		//if (Subsystem)
		//{
		//	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		//	if (SessionInterface.IsValid())
		//	{
		//		auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
		//		if (ExistingSession)
		//		{
		//			FOnlineSessionSettings UpdatedSessionSettings = ExistingSession->SessionSettings;
		//			UpdatedSessionSettings.Set(FName("SessionDisplayName"), FString("CHANGED"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		//			SessionInterface->UpdateSession(NAME_GameSession, UpdatedSessionSettings, true);
		//		}
		//	}
		//}

	
}

// none of the parameters are null
void ABlasterGameMode::PlayerKilled(ABlasterCharacter* KilledPlayer, ABlasterPlayerController* KilledPlayerController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* KilledPlayerState = KilledPlayerController ? Cast<ABlasterPlayerState>(KilledPlayerController->PlayerState) : nullptr;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (KilledPlayer->IsKilled())
	{
		return;
	}
	if (AttackerPlayerState && AttackerPlayerState != KilledPlayerState)
	{
		AttackerPlayerState->ServerAddToScore(1.0f);
		AttackerPlayerState->ServerAddToKills(1);
		BlasterGameState->ServerUpdateTopScore(AttackerPlayerState);
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


