// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlasterFillableBar.generated.h"

/**
 * 
 */
UCLASS()



class BLASTER_API UBlasterFillableBar : public UUserWidget
{
	GENERATED_BODY()
	

public:

	// Percentage between 0-1. Exponential - Used for easing, degree of the exponential curve
	UFUNCTION(BlueprintCallable)
	void StartPercentageChange(float NewPercentage, float PercentageChangeExponential, float PercentageChangeDuration);

	UPROPERTY(meta = (BindWidget))
	class UBorder* FullBar;

	UPROPERTY(meta = (BindWidget))
	UBorder* TemporaryBar;

	UPROPERTY(meta = (BindWidget))
	UBorder* EmptyBar;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	class UHorizontalBoxSlot* FullBarSlot = nullptr;
	UHorizontalBoxSlot* EmptyBarSlot = nullptr;
	UHorizontalBoxSlot* TemporaryBarSlot = nullptr;

	float CurrentPercentage = 1.0f;
	float TargetPercentage = 1.0f;
	// Exponential use for easing, parameter
	float CurrentPercentageChangeExponential = 1.0f;
	// Parameter
	float CurrentPercentageChangeDuration = 1.0f;
	// Between 0-1 how much of the transition have we done
	float PercentageChangeProgress = 0.0f;

	// This will be called per tick
	void UpdateBars();

	// Change individual bar's horizontal box slot size (fill)
	void ChangeBarSize(UHorizontalBoxSlot* Slot, float Size);
};
