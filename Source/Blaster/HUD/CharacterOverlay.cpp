// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"
#include "Components/TextBlock.h"
#include "Blaster/HUD/BlasterFillableBar.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Animation/WidgetAnimation.h"

void UCharacterOverlay::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ResetHUD(); // HUD in editor is preview and contains testing values
	// Start with shop icon hidden
	PlayAnimation(ShopIconAnimationIn, ShopIconAnimationIn->GetEndTime(), 1, EUMGSequencePlayMode::Reverse);
}

void UCharacterOverlay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	FShopRelatedWidgetSelected PreviousSelectedWidget = ShopRelatedWidgetSelected;
	if (MainWeapon->IsHovered())
	{
		ShopRelatedWidgetSelected = FShopRelatedWidgetSelected::MainWeapon;
	}
	else if (SecondaryWeapon->IsHovered())
	{
		ShopRelatedWidgetSelected = FShopRelatedWidgetSelected::SecondaryWeapon;
	}
	else if (Item1->IsHovered())
	{
		ShopRelatedWidgetSelected = FShopRelatedWidgetSelected::Item1;
	}
	else if (Item2->IsHovered())
	{
		ShopRelatedWidgetSelected = FShopRelatedWidgetSelected::Item2;
	}
	else if (Ability->IsHovered())
	{
		ShopRelatedWidgetSelected = FShopRelatedWidgetSelected::Ability;
	}
	else
	{
		ShopRelatedWidgetSelected = FShopRelatedWidgetSelected::None;
	}
	if (ShopRelatedWidgetSelected != PreviousSelectedWidget)
	{
		OnShopRelatedWidgetSelectionChange();
	}

	Super::NativeTick(MyGeometry, InDeltaTime);
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

void UCharacterOverlay::ShowShopIcon(bool bShow)
{
	if (bShow)
	{
		PlayAnimation(ShopIconAnimationIn, 0.0f);
	}
	else
	{
		PlayAnimation(ShopIconAnimationIn, 0.0f, 1, EUMGSequencePlayMode::Reverse);
	}
}

void UCharacterOverlay::ShowShop(bool bShow)
{

}

void UCharacterOverlay::OnShopRelatedWidgetSelectionChange()
{
	if (ShopRelatedWidgetSelected == FShopRelatedWidgetSelected::MainWeapon)
	{
		
	}
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
