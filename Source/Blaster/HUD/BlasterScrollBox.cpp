// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBoxSlot.h"

void UBlasterScrollBox::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ItemsBox)
	{
		if (bTopToBottom)
		{
			Cast<UHorizontalBoxSlot>(ItemsBox->Slot)->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		}
		else
		{
			Cast<UHorizontalBoxSlot>(ItemsBox->Slot)->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		}
		
	}
}
