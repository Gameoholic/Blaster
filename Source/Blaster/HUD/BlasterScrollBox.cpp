// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/Border.h"

void UBlasterScrollBox::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Change all widgets according to parameters
	if (ItemsBox)
	{
		if (bTopToBottom)
		{
			Cast<UHorizontalBoxSlot>(ItemsBox->Slot)->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
			ItemsBox->SetRenderTransformAngle(0); // So the order will be top to bottom, with bottom ones clipping out
			//AND ALL CHILDREN WILL BE 0 AS WELL
		}
		else
		{
			Cast<UHorizontalBoxSlot>(ItemsBox->Slot)->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
			ItemsBox->SetRenderTransformAngle(180); // So the order will be bottom to top, with top ones clipping out
			// AND ALL CHILDREN WILL BE 180 AS WELL
		}
	}

	if (ScrollWheel)
	{
		ScrollWheel->SetPadding(FMargin(ScrollWheelSize, 0.0f, 0.0f, 0.0f));
	}

	if (bAlwaysShowScrollbar)
	{
		ScrollWheel->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ScrollWheel->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UBlasterScrollBox::AddChild(UWidget* WidgetToAdd)
{
	InternalAddChild(WidgetToAdd);
	DisplayChildren();
}

void UBlasterScrollBox::AddChildren(TArray<UWidget*> WidgetsToAdd)
{
	for (UWidget* Widget : WidgetsToAdd)
	{
		InternalAddChild(Widget);
	}
	DisplayChildren();
}

void UBlasterScrollBox::InternalAddChild(UWidget* WidgetToAdd)
{
	bTopToBottom ? WidgetToAdd->SetRenderTransformAngle(0) : WidgetToAdd->SetRenderTransformAngle(180); // Needs to match angle of ItemsBox
	OriginalChildren.Add(WidgetToAdd);
}

void UBlasterScrollBox::DisplayChildren()
{
	ItemsBox->ClearChildren();
	if (bReverseOrder)
	{
		for (int32 i = OriginalChildren.Num() - 1; i >= 0; i--)
		{
			ItemsBox->AddChild(OriginalChildren[i]);
		}
	}
	else
	{
		for (int32 i = 0; i < OriginalChildren.Num(); i++)
		{
			ItemsBox->AddChild(OriginalChildren[i]);
		}
	}
}


