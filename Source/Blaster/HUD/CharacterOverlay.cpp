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
	if (bShopOpened)
	{
		HandleOpenedShopTick(InDeltaTime);
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

void UCharacterOverlay::HandleOpenedShopTick(float DeltaTime)
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

	TickShopRelatedWidgetColors(DeltaTime);
	TickSelectedShopRelatedWidget(DeltaTime);
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
		SetAllShopRelatedWidgetsColor(Color);
	}
}

void UCharacterOverlay::OnShopRelatedWidgetSelectionChange(FShopRelatedWidgetSelected PreviousWidget)
{
	GetIconFromSelectedShopRelatedWidget(PreviousWidget)->SetRenderTranslation(FVector2D(0.0f, 0.0f));
}

void UCharacterOverlay::TickShopRelatedWidgetColors(float DeltaTime)
{
	const float RateOfChange = 75.0f;
	ShopRelatedWidgetColorChangeTime += RateOfChange * DeltaTime;
	ShopRelatedWidgetColorChangeTime = ShopRelatedWidgetColorChangeTime >= 360.0f ? ShopRelatedWidgetColorChangeTime - 360.0f : ShopRelatedWidgetColorChangeTime;
	FLinearColor Color = FLinearColor(ShopRelatedWidgetColorChangeTime, 1.0f, 1.0f); // In HSV
	SetAllShopRelatedWidgetsColor(Color);
}

void UCharacterOverlay::TickSelectedShopRelatedWidget(float DeltaTime)
{
	const float ShakeDelay = 0.1f;
	ShopRelatedWidgetShakeTime += DeltaTime;
	if (ShopRelatedWidgetShakeTime < ShakeDelay)
	{
		return;
	}
	ShopRelatedWidgetShakeTime = 0.0f;


	const float ShakeAmplitude = 10.0f;
	GetIconFromSelectedShopRelatedWidget()->SetRenderTranslation(FVector2D(
		FMath::RandRange(-ShakeAmplitude, ShakeAmplitude), 
		FMath::RandRange(-ShakeAmplitude, ShakeAmplitude)
	));
}

UImage* UCharacterOverlay::GetIconFromSelectedShopRelatedWidget(FShopRelatedWidgetSelected Widget) const
{
	switch (Widget)
	{
	case FShopRelatedWidgetSelected::MainWeapon:
		return MainWeaponIcon;
	case FShopRelatedWidgetSelected::SecondaryWeapon:
		return SecondaryWeaponIcon;
	case FShopRelatedWidgetSelected::Item1:
		return Item1Icon;
	case FShopRelatedWidgetSelected::Item2:
		return Item2Icon;
	case FShopRelatedWidgetSelected::Ability:
		return AbilityIcon;
	}
	return nullptr;
}



void UCharacterOverlay::SetAllShopRelatedWidgetsColor(FLinearColor HSVColor)
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
