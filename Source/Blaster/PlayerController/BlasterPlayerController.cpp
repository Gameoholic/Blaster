// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/GameState/BlasterGameState.h"



void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ServerCheckMatchState();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();

	CheckTimeSync(DeltaTime);

	PollInit();
}



void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncElapsedSeconds += DeltaTime;
	if (IsLocalController() && TimeSyncElapsedSeconds > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncElapsedSeconds = 0.0f;
	}
}



void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}



void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDeaths(HUDDeaths);
			}
		}
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime);

		if (BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnnouncement();
		}
	}
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float ProvidedWarmupTime, float ProvidedMatchTime, float ProvidedStartingTime)
{
	WarmupTime = ProvidedWarmupTime;
	MatchTime = ProvidedMatchTime;
	LevelStartingTime = ProvidedStartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

bool ABlasterPlayerController::IsHUDValid()
{
	BlasterHUD = (BlasterHUD == nullptr && GetHUD() != nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	return (BlasterHUD != nullptr && BlasterHUD->CharacterOverlay != nullptr &&
		BlasterHUD->CharacterOverlay->HealthBar != nullptr && BlasterHUD->CharacterOverlay->HealthText != nullptr
		&& BlasterHUD->CharacterOverlay->ScoreText != nullptr && BlasterHUD->CharacterOverlay->WeaponAmmoText != nullptr
		&& BlasterHUD->CharacterOverlay->MatchCountdownText != nullptr && BlasterHUD->Announcement != nullptr && BlasterHUD->Announcement->WarmupTime != nullptr);
}




void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	if (!IsHUDValid())
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
		return;
	}

	const float HealthPercent = Health / MaxHealth;
	BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
	BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	if (!IsHUDValid())
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
		return;
	}
	FString ScoreText = FString::Printf(TEXT("Score: %d"), FMath::CeilToInt(Score));
	BlasterHUD->CharacterOverlay->ScoreText->SetText(FText::FromString(ScoreText));
}

void ABlasterPlayerController::SetHUDKills(int32 Kills)
{
	if (!IsHUDValid())
	{
		return;
	}
	FString KillsText = FString::Printf(TEXT("Kills: %d"), Kills);
	BlasterHUD->CharacterOverlay->KillsText->SetText(FText::FromString(KillsText));
}

void ABlasterPlayerController::SetHUDDeaths(int32 Deaths)
{
	if (!IsHUDValid())
	{
		bInitializeCharacterOverlay = true;
		HUDDeaths = Deaths;
		return;
	}
	FString DeathsText = FString::Printf(TEXT("Deaths: %d"), Deaths);
	BlasterHUD->CharacterOverlay->DeathsText->SetText(FText::FromString(DeathsText));
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 WeaponAmmo)
{
	if (!IsHUDValid())
	{
		return;
	}
	FString AmmoText = FString::Printf(TEXT("%d"), WeaponAmmo);
	BlasterHUD->CharacterOverlay->WeaponAmmoText->SetText(FText::FromString(AmmoText));
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	if (!IsHUDValid())
	{
		return;
	}
	FString CarriedAmmoText = FString::Printf(TEXT("%d"), CarriedAmmo);
	BlasterHUD->CharacterOverlay->CarriedWeaponAmmoText->SetText(FText::FromString(CarriedAmmoText));
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	if (!IsHUDValid())
	{
		return;
	}

	int32 Minutes = FMath::FloorToInt(CountdownTime / 60.0f);
	int32 Seconds = CountdownTime - Minutes * 60;

	FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	if (!IsHUDValid())
	{
		return;
	}

	int32 Minutes = FMath::FloorToInt(CountdownTime / 60.0f);
	int32 Seconds = CountdownTime - Minutes * 60;

	FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}




void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.0f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDAnnouncementCountdown(TimeLeft);

		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float CurrentServerTime = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, CurrentServerTime);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + 0.5f * RoundTripTime;

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}


float ABlasterPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}


void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;


	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}



void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
