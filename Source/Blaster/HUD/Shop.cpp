// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/BlasterScrollBox.h"
#include "Blaster/HUD/ShopPurchasable.h"
#include "Blaster/Weapons/Weapon.h"
#include "Blueprint/WidgetTree.h"


void UShop::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UShop::DisplayPurchasables(FShopRelatedWidget Category)
{
	Purchasables->RemoveAllChildren();
	if (!ShopPurchasableClass)
	{
		return;
	}

	if (Category == FShopRelatedWidget::MainWeapon || Category == FShopRelatedWidget::SecondaryWeapon)
	{
		for (TSubclassOf<AWeapon> WeaponClass : PurchasableWeapons)
		{
			AWeapon* Weapon = Cast<AWeapon>(WeaponClass->GetDefaultObject());
			UShopPurchasable* Purchasable = CreateWidget<UShopPurchasable>(this, ShopPurchasableClass);
			Purchasable->SetPurchasableValues(Weapon->GetDisplayName(), Weapon->GetIcon(), Weapon->GetCost());
			Purchasables->AddChild(Purchasable);
		}
	}
	else if (Category == FShopRelatedWidget::Ability)
	{
		for (TSubclassOf<AWeapon> WeaponClass : PurchasableWeapons2Temp)
		{
			AWeapon* Weapon = Cast<AWeapon>(WeaponClass->GetDefaultObject());
			UShopPurchasable* Purchasable = CreateWidget<UShopPurchasable>(this, ShopPurchasableClass);
			Purchasable->SetPurchasableValues(Weapon->GetDisplayName(), Weapon->GetIcon(), Weapon->GetCost());
			Purchasables->AddChild(Purchasable);
		}
	}
	Purchasables->SnapScrollBar(true);
}


void UShop::DisplayNextPurchasable()
{

}