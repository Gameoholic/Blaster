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
#include "Components/Border.h"

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

	if (bShopOpened)
	{
		ChangeShopIconsColors(InDeltaTime);
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
	bShopOpened = bShow;
	if (bShow)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		//InputModeData.SetWidgetToFocus(Shop->TakeWidget());
		GetOwningPlayer()->SetInputMode(InputModeData);
		GetOwningPlayer()->SetShowMouseCursor(true);
	}
	else
	{
		FInputModeGameOnly InputModeData;
		GetOwningPlayer()->SetInputMode(InputModeData);
		GetOwningPlayer()->SetShowMouseCursor(false);

		FLinearColor Color = FLinearColor(0.0f, 0.0f, 1.0f); // In HSV
		SetAllShopIconsColors(Color);
	}
}

void UCharacterOverlay::OnShopRelatedWidgetSelectionChange()
{
	if (ShopRelatedWidgetSelected == FShopRelatedWidgetSelected::MainWeapon)
	{
		
	}
}

void UCharacterOverlay::ChangeShopIconsColors(float DeltaTime)
{
	const float RateOfChange = 75.0f;
	ShopIconColorChangeTime += RateOfChange * DeltaTime;
	ShopIconColorChangeTime = ShopIconColorChangeTime >= 360.0f ? ShopIconColorChangeTime - 360.0f : ShopIconColorChangeTime;
	FLinearColor Color = FLinearColor(ShopIconColorChangeTime, 1.0f, 1.0f); // In HSV
	SetAllShopIconsColors(Color);
}

void UCharacterOverlay::SetAllShopIconsColors(FLinearColor HSVColor)
{
	MainWeaponIcon->SetBrushTintColor(HSVColor.HSVToLinearRGB());
	MainWeaponBorder->SetBrushColor(HSVColor.HSVToLinearRGB());
	SecondaryWeaponIcon->SetBrushTintColor(HSVColor.HSVToLinearRGB());
	SecondaryWeaponBorder->SetBrushColor(HSVColor.HSVToLinearRGB());
	AbilityIcon->SetBrushTintColor(HSVColor.HSVToLinearRGB());
	AbilityBorder->SetBrushColor(HSVColor.HSVToLinearRGB());
	Item1Icon->SetBrushTintColor(HSVColor.HSVToLinearRGB());
	Item1Border->SetBrushColor(HSVColor.HSVToLinearRGB());
	Item2Icon->SetBrushTintColor(HSVColor.HSVToLinearRGB());
	Item2Border->SetBrushColor(HSVColor.HSVToLinearRGB());
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
