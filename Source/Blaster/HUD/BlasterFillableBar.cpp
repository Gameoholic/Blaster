// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterFillableBar.h"
#include "Components/Border.h"
#include "Components/HorizontalBoxSlot.h"




void UBlasterFillableBar::NativeOnInitialized()
{
	Super::OnInitialized();

	FullBarSlot = Cast<UHorizontalBoxSlot>(FullBar->Slot);
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

		SetPercentage();
	}

}

void UBlasterFillableBar::StartPercentageChange(float NewPercentage, float PercentageChangeExponential, float PercentageChangeDuration)
{
	PercentageChangeProgress = 0.0f;
	CurrentPercentageChangeExponential = PercentageChangeExponential;
	CurrentPercentageChangeDuration = PercentageChangeDuration;
	TargetPercentage = NewPercentage;
}


void UBlasterFillableBar::SetPercentage()
{
	if (!FullBarSlot || !EmptyBarSlot)
	{
		return;
	}

	FSlateChildSize FullBarSlotSize = FSlateChildSize(ESlateSizeRule::Fill);
	FullBarSlotSize.Value = CurrentPercentage;

	FSlateChildSize EmptyBarSlotSize = FSlateChildSize(ESlateSizeRule::Fill);
	EmptyBarSlotSize.Value = 1.0f - CurrentPercentage;

	FullBarSlot->SetSize(FullBarSlotSize);
	EmptyBarSlot->SetSize(EmptyBarSlotSize);
}

