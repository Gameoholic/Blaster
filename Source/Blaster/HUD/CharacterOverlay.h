// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UChat;
class UShop;
class UWidgetAnimation;
class USizeBox;
class UImage;
class UCanvasPanel;
class UBorder;

UENUM()
enum class FShopRelatedWidget : uint8
{
	None UMETA(DisplayName = "None"),
	MainWeapon UMETA(DisplayName = "MainWeapon"),
	SecondaryWeapon UMETA(DisplayName = "SecondaryWeapon"),
	Item1 UMETA(DisplayName = "Item1"),
	Item2 UMETA(DisplayName = "Item2"),
	Ability UMETA(DisplayName = "Ability"),

	MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	

public:
	bool IsHUDValid();

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* CanvasPanel;

	UPROPERTY(meta = (BindWidget))
	class UBlasterFillableBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CurrentHealthText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MaxHealthText;

	UPROPERTY(meta = (BindWidget))
	class UBlasterFillableBar* AmmoBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CurrentAmmoText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MaxAmmoText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathsText;


	UPROPERTY(meta = (BindWidget))
	USizeBox* MainWeapon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MainWeaponName;

	UPROPERTY(meta = (BindWidget))
	UImage* MainWeaponIcon;

	UPROPERTY(meta = (BindWidget))
	UBorder* MainWeaponBorder;

	UPROPERTY(meta = (BindWidget))
	USizeBox* SecondaryWeapon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SecondaryWeaponName;

	UPROPERTY(meta = (BindWidget))
	UImage* SecondaryWeaponIcon;

	UPROPERTY(meta = (BindWidget))
	UBorder* SecondaryWeaponBorder;

	UPROPERTY(meta = (BindWidget))
	USizeBox* Ability;

	UPROPERTY(meta = (BindWidget))
	UImage* AbilityIcon;

	UPROPERTY(meta = (BindWidget))
	UBorder* AbilityBorder;

	UPROPERTY(meta = (BindWidget))
	USizeBox* Item1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Item1Name;

	UPROPERTY(meta = (BindWidget))
	UImage* Item1Icon;

	UPROPERTY(meta = (BindWidget))
	UBorder* Item1Border;

	UPROPERTY(meta = (BindWidget))
	USizeBox* Item2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Item2Name;

	UPROPERTY(meta = (BindWidget))
	UImage* Item2Icon;

	UPROPERTY(meta = (BindWidget))
	UBorder* Item2Border;

	UPROPERTY(meta = (BindWidget))
	UChat* Chat;

	UPROPERTY(meta = (BindWidget))
	UShop* Shop;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ShopIconAnimationIn;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ShopAnimationIn;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* NotEnoughMoneyAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoneyAmount;

	void ShowShopIcon(bool bShow);
	void ShowShop(bool bShow);
	void SetMoney(int32 Money);
	void PlayNotEnoughMoneyAnimation();
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	void ResetHUD();

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	FSlateBrush InvisibleImage;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	FSlateBrush ShopLineMaterial;

	// Shop:
	void HandleOpenedShopTick(float DeltaTime);
	bool bShopOpened = false;

	FShopRelatedWidget SelectedShopRelatedWidget = FShopRelatedWidget::None;
	void OnShopRelatedWidgetSelectionChange(FShopRelatedWidget PreviousWidget);

	float ShopRelatedWidgetColorChangeTime = 0.0f;
	void TickShopRelatedWidgetColors(float DeltaTime);
	void SetAllShopRelatedWidgetsColor(FLinearColor HSVColor);

	float ShopRelatedWidgetShakeTime = 0.0f;
	void TickSelectedShopRelatedWidget(float DeltaTime);

	UImage* GetIconFromSelectedShopRelatedWidget(FShopRelatedWidget Widget) const;
	UBorder* GetBorderFromSelectedShopRelatedWidget(FShopRelatedWidget Widget) const;

};
