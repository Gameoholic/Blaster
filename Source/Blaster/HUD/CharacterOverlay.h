// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UChat;

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	

public:
	bool IsHUDValid();


	UPROPERTY(EditDefaultsOnly)
	FSlateBrush InvisibleImage;

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
	UTextBlock* MainWeaponName;

	UPROPERTY(meta = (BindWidget))
	class UImage* MainWeaponIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SecondaryWeaponName;

	UPROPERTY(meta = (BindWidget))
	UImage* SecondaryWeaponIcon;



	UPROPERTY(meta = (BindWidget))
	UTextBlock* Item1Name;

	UPROPERTY(meta = (BindWidget))
	class UImage* Item1Icon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Item2Name;

	UPROPERTY(meta = (BindWidget))
	UImage* Item2Icon;

	UPROPERTY(meta = (BindWidget))
	UChat* Chat;

protected:
	virtual void NativeOnInitialized() override;

private:
	void ResetHUD();
};
