// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

bool ABlasterPlayerController::IsHUDValid()
{
	BlasterHUD = (BlasterHUD == nullptr && GetHUD() != nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	return (BlasterHUD != nullptr && BlasterHUD->CharacterOverlay != nullptr &&
		BlasterHUD->CharacterOverlay->HealthBar != nullptr && BlasterHUD->CharacterOverlay->HealthText != nullptr
		&& BlasterHUD->CharacterOverlay->ScoreText != nullptr && BlasterHUD->CharacterOverlay->WeaponAmmoText != nullptr);
}


void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	if (!IsHUDValid())
	{
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

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}
