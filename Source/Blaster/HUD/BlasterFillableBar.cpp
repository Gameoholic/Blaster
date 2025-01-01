// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterFillableBar.h"
#include "Components/Border.h"
#include "Components/HorizontalBoxSlot.h"




void UBlasterFillableBar::NativeOnInitialized()
{
	Super::OnInitialized();

	FullBarSlot = Cast<UHorizontalBoxSlot>(FullBar->Slot);
	TemporaryBarSlot = Cast<UHorizontalBoxSlot>(TemporaryBar->Slot);
	EmptyBarSlot = Cast<UHorizontalBoxSlot>(EmptyBar->Slot);

	StartPercentageChange(0.0f, 0.0f, 0.0f); // Reset percentage to default value 1.0f
	// TODO: Have the default value be customizable
}

void UBlasterFillableBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (PercentageChangeProgress < 1.0f)
	{
		PercentageChangeProgress += InDeltaTime / CurrentPercentageChangeDuration;
		PercentageChangeProgress = FMath::Clamp(PercentageChangeProgress, 0.0f, 1.0f);

		CurrentPercentage = FMath::InterpEaseInOut(
			CurrentPercentage,
			TargetPercentage,
			PercentageChangeProgress,
			CurrentPercentageChangeExponential
		);

		UpdateBars();
	}

}

void UBlasterFillableBar::StartPercentageChange(float NewPercentage, float PercentageChangeExponential, float PercentageChangeDuration)
{
	if (!FullBarSlot || !TemporaryBarSlot || !EmptyBarSlot)
	{
		return;
	}

	// Reset & set all variables for this percentage change
	PercentageChangeProgress = 0.0f;
	CurrentPercentageChangeExponential = PercentageChangeExponential;
	CurrentPercentageChangeDuration = PercentageChangeDuration;
	TargetPercentage = NewPercentage;

	// Set full bar size to target percentage immediately
	ChangeBarSize(FullBarSlot, TargetPercentage);
	// The removed bar chunk will be replaced by the temporary bar
	ChangeBarSize(TemporaryBarSlot, CurrentPercentage - TargetPercentage);
	// Empty bar remains the same
}

// full bar: 0.9
// temp: 0.0
// empty: 0.1

// full: 0.5 (-target percentage)
// temp: 0.4 (=target percentage) ...-> 0.0
// empty: 0.1 ...-> 0.5 (+ target percentage)


void UBlasterFillableBar::UpdateBars()
{
	if (!FullBarSlot || !TemporaryBarSlot || !EmptyBarSlot)
	{
		return;
	}

	// Temporary bar shrinks to the full bar size for the duration of the animation
	ChangeBarSize(TemporaryBarSlot, CurrentPercentage - TargetPercentage);
	// Empty bar grows and replaces the temporary bar for the duration of the animation
	ChangeBarSize(EmptyBarSlot, 1.0 - CurrentPercentage);
	// Full bar remains the same
}

void UBlasterFillableBar::ChangeBarSize(UHorizontalBoxSlot* SlotParam, float Size)
{
	FSlateChildSize SlotSize = FSlateChildSize(ESlateSizeRule::Fill);
	SlotSize.Value = Size;
	SlotParam->SetSize(SlotSize);
}

