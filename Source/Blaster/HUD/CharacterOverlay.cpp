// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"
#include "Components/TextBlock.h"
#include "Blaster/HUD/BlasterFillableBar.h"
#include "Components/Image.h"


void UCharacterOverlay::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ResetHUD(); // HUD in editor is preview and contains testing values
}

void UCharacterOverlay::ResetHUD()
{
	CurrentHealthText->SetText(FText::FromString("100"));
	MaxHealthText->SetText(FText::FromString("100"));
	HealthBar->StartPercentageChange(1.0f, 0.0f, 0.0f);

	CurrentAmmoText->SetText(FText::FromString("0"));
	MaxAmmoText->SetText(FText::FromString("0"));
	AmmoBar->StartPercentageChange(0.0f, 0.0f, 0.0f);

	MainWeaponIcon->SetBrush(InvisibleImage);
	MainWeaponName->SetText(FText::FromString(""));

	SecondaryWeaponIcon->SetBrush(InvisibleImage);
	SecondaryWeaponName->SetText(FText::FromString(""));

	Item1Icon->SetBrush(InvisibleImage);
	Item1Name->SetText(FText::FromString(""));

	Item2Icon->SetBrush(InvisibleImage);
	Item2Name->SetText(FText::FromString(""));
}

bool UCharacterOverlay::IsHUDValid()
{
	return (
		HealthBar != nullptr
		&& ScoreText != nullptr && AmmoBar != nullptr
		&& CurrentAmmoText != nullptr && MaxAmmoText != nullptr
		&& CurrentHealthText != nullptr &&
		MaxHealthText != nullptr &&
		MainWeaponName != nullptr &&
		MainWeaponIcon != nullptr &&
		SecondaryWeaponName != nullptr &&
		SecondaryWeaponIcon != nullptr &&
		Item1Name != nullptr &&
		Item1Icon != nullptr &&
		Item2Name != nullptr &&
		Item2Icon != nullptr &&
		Chat != nullptr);
}
