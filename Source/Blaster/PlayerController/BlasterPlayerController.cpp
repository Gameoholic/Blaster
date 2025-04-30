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
#include "Blaster/HUD/PauseMenu.h"
#include "Blaster/HUD/BlasterFillableBar.h"
#include "Components/Image.h"
#include "Blaster/HUD/Chat.h"
#include "Components/EditableText.h"
#include "Blaster/GameInstance/BlasterGameInstance.h"
#include "Components/SizeBox.h"



void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//ServerCheckMatchState();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	//ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::ClientSendMessage_Implementation(FName Message)
{
	SendChatMessage(Message);
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//SetHUDTime();

	//CheckTimeSync(DeltaTime);

	PollInit();
}



//void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
//{
//	TimeSyncElapsedSeconds += DeltaTime;
//	if (IsLocalController() && TimeSyncElapsedSeconds > TimeSyncFrequency)
//	{
//		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
//		TimeSyncElapsedSeconds = 0.0f;
//	}
//}
//
//
//
//void ABlasterPlayerController::ReceivedPlayer()
//{
//	Super::ReceivedPlayer();
//
//	if (IsLocalController())
//	{
//		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
//	}
//}



void ABlasterPlayerController::PollInit()
{
	// Load chat messages
	if (!bChatMessagesLoadedFromGameInstance && IsHUDValid())
	{
		UBlasterGameInstance* GameInstance = Cast<UBlasterGameInstance>(GetGameInstance());
		if (GameInstance)
		{
			BlasterHUD->CharacterOverlay->Chat->AddMessagesSilently(GameInstance->GetChatMessages());
			bChatMessagesLoadedFromGameInstance = true;
		}
	}
}

//void ABlasterPlayerController::ServerCheckMatchState_Implementation()
//{
//	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
//	if (GameMode)
//	{
//		WarmupTime = GameMode->WarmupTime;
//		MatchTime = GameMode->MatchTime;
//		LevelStartingTime = GameMode->LevelStartingTime;
//		MatchState = GameMode->GetMatchState();
//		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime);
//
//		if (BlasterHUD && MatchState == MatchState::WaitingToStart)
//		{
//			BlasterHUD->AddAnnouncement();
//		}
//	}
//}
//
//void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float ProvidedWarmupTime, float ProvidedMatchTime, float ProvidedStartingTime)
//{
//	WarmupTime = ProvidedWarmupTime;
//	MatchTime = ProvidedMatchTime;
//	LevelStartingTime = ProvidedStartingTime;
//	MatchState = StateOfMatch;
//	OnMatchStateSet(MatchState);
//	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
//	{
//		BlasterHUD->AddAnnouncement();
//	}
//}

bool ABlasterPlayerController::IsHUDValid()
{
	BlasterHUD = (BlasterHUD == nullptr && GetHUD() != nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD == nullptr || BlasterHUD->CharacterOverlay == nullptr)
	{
		return false;
	}
	return BlasterHUD->CharacterOverlay->IsHUDValid();
}


void ABlasterPlayerController::SetHUDHealth(float PreviousHealth, float Health, float MaxHealth)
{
	if (!IsHUDValid())
	{
		return;
	}

	const float HealthPercent = Health / MaxHealth;
	// If damage was more than 50% of max health
	if (PreviousHealth - Health >= MaxHealth / 2.0f)
	{
		BlasterHUD->CharacterOverlay->HealthBar->StartPercentageChange(HealthPercent, 1.4f, 3.0f);
	}
	else
	{
		BlasterHUD->CharacterOverlay->HealthBar->StartPercentageChange(HealthPercent, 0.5f, 1.0f);
	}
	BlasterHUD->CharacterOverlay->CurrentHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::CeilToInt32(Health))));
	BlasterHUD->CharacterOverlay->MaxHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::CeilToInt32(MaxHealth))));
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 WeaponRemainingAmmo, int32 WeaponMagCapacity)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->CurrentAmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d"), WeaponRemainingAmmo)));
	BlasterHUD->CharacterOverlay->MaxAmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d"), WeaponMagCapacity)));

	const float AmmoPercent = (float)WeaponRemainingAmmo / WeaponMagCapacity;
	BlasterHUD->CharacterOverlay->AmmoBar->StartPercentageChange(AmmoPercent, 1.4f, 1.0f);
}

void ABlasterPlayerController::SetHUDMainWeapon(FString WeaponName, FSlateBrush WeaponIcon)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->MainWeaponName->SetText(FText::FromString(WeaponName));
	BlasterHUD->CharacterOverlay->MainWeaponIcon->SetBrush(WeaponIcon);
}

void ABlasterPlayerController::SetHUDSecondaryWeapon(FString WeaponName, FSlateBrush WeaponIcon)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->SecondaryWeaponName->SetText(FText::FromString(WeaponName));
	BlasterHUD->CharacterOverlay->SecondaryWeaponIcon->SetBrush(WeaponIcon);
}

void ABlasterPlayerController::SetHUDItem1(FString ItemName, FSlateBrush ItemIcon)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->Item1Name->SetText(FText::FromString(ItemName));
	BlasterHUD->CharacterOverlay->Item1Icon->SetBrush(ItemIcon);
}

void ABlasterPlayerController::SetHUDItem2(FString ItemName, FSlateBrush ItemIcon)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->Item2Name->SetText(FText::FromString(ItemName));
	BlasterHUD->CharacterOverlay->Item2Icon->SetBrush(ItemIcon);
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	if (!IsHUDValid())
	{
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
		return;
	}
	FString DeathsText = FString::Printf(TEXT("Deaths: %d"), Deaths);
	BlasterHUD->CharacterOverlay->DeathsText->SetText(FText::FromString(DeathsText));
}

void ABlasterPlayerController::ToggleChat()
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->Chat->ToggleChat();
}

void ABlasterPlayerController::SendChatMessage(FName Message)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->Chat->ReceiveMessage(Message);
}

void ABlasterPlayerController::SetHUDShowShopIcon(bool bShow)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->ShowShopIcon(bShow);
}

void ABlasterPlayerController::ShowShop(bool bShow)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->ShowShop(bShow);
}

void ABlasterPlayerController::SetHUDMoney(int32 Money)
{
	if (!IsHUDValid())
	{
		return;
	}
	BlasterHUD->CharacterOverlay->SetMoney(Money);
}

//void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
//{
//	if (!IsHUDValid())
//	{
//		return;
//	}
//
//	int32 Minutes = FMath::FloorToInt(CountdownTime / 60.0f);
//	int32 Seconds = CountdownTime - Minutes * 60;
//
//	FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
//	BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
//}
//
//void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
//{
//	if (!IsHUDValid())
//	{
//		return;
//	}
//
//	int32 Minutes = FMath::FloorToInt(CountdownTime / 60.0f);
//	int32 Seconds = CountdownTime - Minutes * 60;
//
//	FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
//	BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
//}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}




//void ABlasterPlayerController::SetHUDTime()
//{
//	float TimeLeft = 0.0f;
//	if (MatchState == MatchState::WaitingToStart)
//	{
//		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
//	}
//	else if (MatchState == MatchState::InProgress)
//	{
//		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
//	}
//
//	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
//	if (CountdownInt != SecondsLeft)
//	{
//		if (MatchState == MatchState::WaitingToStart)
//		{
//			SetHUDAnnouncementCountdown(TimeLeft);
//
//		}
//		else if (MatchState == MatchState::InProgress)
//		{
//			SetHUDMatchCountdown(TimeLeft);
//		}
//	}
//
//	CountdownInt = SecondsLeft;
//}
//
//void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
//{
//	float CurrentServerTime = GetWorld()->GetTimeSeconds();
//	ClientReportServerTime(TimeOfClientRequest, CurrentServerTime);
//}
//
//void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
//{
//	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
//	float CurrentServerTime = TimeServerReceivedClientRequest + 0.5f * RoundTripTime;
//
//	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
//}
//
//
//float ABlasterPlayerController::GetServerTime()
//{
//	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
//}
//
//
//void ABlasterPlayerController::OnMatchStateSet(FName State)
//{
//	MatchState = State;
//
//
//	if (MatchState == MatchState::InProgress)
//	{
//		HandleMatchHasStarted();
//	}
//	else if (MatchState == MatchState::Cooldown)
//	{
//		HandleCooldown();
//	}
//}
//
//
//
//void ABlasterPlayerController::OnRep_MatchState()
//{
//	if (MatchState == MatchState::InProgress)
//	{
//		HandleMatchHasStarted();
//	}
//	else if (MatchState == MatchState::Cooldown)
//	{
//		HandleCooldown();
//	}
//}
//
//void ABlasterPlayerController::HandleMatchHasStarted()
//{
//	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
//	if (BlasterHUD)
//	{
//		BlasterHUD->AddCharacterOverlay();
//		if (BlasterHUD->Announcement)
//		{
//			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
//		}
//	}
//}
//
//void ABlasterPlayerController::HandleCooldown()
//{
//	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
//	if (BlasterHUD)
//	{
//		BlasterHUD->CharacterOverlay->RemoveFromParent();
//		if (BlasterHUD->Announcement)
//		{
//			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
//		}
//	}
//}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr)
	{
		return;
	}

	InputComponent->BindAction("Menu", IE_Pressed, this, &ABlasterPlayerController::TogglePauseMenu);
}


void ABlasterPlayerController::TogglePauseMenu()
{
	if (PauseMenuWidget == nullptr)
	{
		return;
	}

	UPauseMenu* PauseMenu = CreateWidget<UPauseMenu>(this, PauseMenuWidget);
	if (PauseMenu)
	{
		PauseMenu->SetupMenu();
	}

}