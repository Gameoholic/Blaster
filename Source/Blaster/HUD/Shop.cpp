// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/BlasterScrollBox.h"
#include "Blaster/HUD/ShopPurchasable.h"
#include "Blaster/Weapons/Weapon.h"
#include "Blueprint/WidgetTree.h"


void UShop::DisplayPurchasables(FShopRelatedWidget Category)
{
	Purchasables->RemoveAllChildren();
	if (!ShopPurchasableClass)
	{
		return;
	}

	if (Category == FShopRelatedWidget::MainWeapon || Category == FShopRelatedWidget::SecondaryWeapon)
	{
		for (TSubclassOf<AWeapon> WeaponSubclass : PurchasableWeapons)
		{
			AWeapon* Weapon = Cast<AWeapon>(WeaponSubclass->GetDefaultObject());
			UShopPurchasable* Purchasable = CreateWidget<UShopPurchasable>(this, ShopPurchasableClass);
			Purchasable->SetPurchasableValues(Weapon->GetDisplayName(), Weapon->GetIcon(), Weapon->GetCost());
			Purchasables->AddChild(Purchasable);
		}

	}


}
