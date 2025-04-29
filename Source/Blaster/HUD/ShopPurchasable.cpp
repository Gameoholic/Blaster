// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopPurchasable.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"



void UShopPurchasable::SetPurchasableValues(FString _Name, FSlateBrush _Icon, int32 _Cost)
{
	Cost = _Cost;

	PurchasableName->SetText(FText::FromString(_Name));
	PurchasableIcon->SetBrush(_Icon);
	PurchasableCost->SetText(FText::FromString(FString::Printf(TEXT("%d"), Cost)));
}

