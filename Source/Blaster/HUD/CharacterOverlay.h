// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	

public:
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


};
