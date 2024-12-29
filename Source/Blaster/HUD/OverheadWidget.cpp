// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"


// called on change level
void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent(); // clean up widget from viewport
	Super::NativeDestruct();
}


void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
		if (TextToDisplay.Len() > 0) // Hidden by default
		{
			TextBorder->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			TextBorder->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}


