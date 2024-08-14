// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Deaths);
	DOREPLIFETIME(ABlasterPlayerState, Kills);
}

// Updates score and displays the new score
void ABlasterPlayerState::ServerAddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount); // score will get rep'd to clients
	DisplayUpdatedScore();
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	DisplayUpdatedScore();
}

// Update the score visually
void ABlasterPlayerState::DisplayUpdatedScore()
{
	if (!IsControllerValid())
	{
		return;
	}
	Controller->SetHUDScore(GetScore());
}

// Updates score and displays the new kills
void ABlasterPlayerState::ServerAddToKills(int KillsAmount)
{
	Kills += KillsAmount; // kills will get rep'd to clients
	DisplayUpdatedKills();
}

void ABlasterPlayerState::OnRep_Kills()
{
	DisplayUpdatedKills();
}

void ABlasterPlayerState::DisplayUpdatedKills()
{
	if (!IsControllerValid())
	{
		return;
	}
	Controller->SetHUDKills(Kills);
}

void ABlasterPlayerState::ServerAddToDeaths(int DeathsAmount)
{
	Deaths += DeathsAmount; // deaths will get rep'd to clients
	DisplayUpdatedDeaths();
}

void ABlasterPlayerState::OnRep_Deaths()
{
	DisplayUpdatedDeaths();
}

void ABlasterPlayerState::DisplayUpdatedDeaths()
{
	if (!IsControllerValid())
	{
		return;
	}
	Controller->SetHUDDeaths(Deaths);
}






bool ABlasterPlayerState::IsControllerValid()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(GetPawn()->GetController()) : Controller;

	return Character && Controller;
}