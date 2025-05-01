// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopPurchasable.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Blaster/HUD/Shop.h"



void UShopPurchasable::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Button->OnClicked.AddUniqueDynamic(this, &ThisClass::OnClicked);
}

void UShopPurchasable::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}



void UShopPurchasable::SetPurchasableValues(UShop* _Shop, FString _Name, FSlateBrush _Icon, int32 _Cost, FShopRelatedWidget _Category, int32 _Index)
{
	Shop = _Shop;
	Cost = _Cost;
	Category = _Category;
	Index = _Index;

	PurchasableName->SetText(FText::FromString(_Name));
	PurchasableIcon->SetBrush(_Icon);
	PurchasableCost->SetText(FText::FromString(FString::Printf(TEXT("%d"), Cost)));
}

void UShopPurchasable::PlayDenyPurchaseAnimation()
{
	PlayAnimation(DenyAnimation);
}

void UShopPurchasable::PlayAcceptPurchaseAnimation()
{
	PlayAnimation(AcceptAnimation);
}

void UShopPurchasable::OnClicked()
{
	Shop->OnPurchasableClicked(this);
}

