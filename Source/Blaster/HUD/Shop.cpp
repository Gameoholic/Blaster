// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/BlasterScrollBox.h"
#include "Blaster/HUD/ShopPurchasable.h"
#include "Blaster/Weapons/Weapon.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/Character.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameState/BlasterGameState.h"


void UShop::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UShop::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UShop::DisplayPurchasables(FShopRelatedWidget Category)
{
	Purchasables->RemoveAllChildren();

	if (!GetWorld())
	{
		return;
	}
	GameState = GameState == nullptr ? Cast<ABlasterGameState>(GetWorld()->GetGameState()) : GameState;
	if (!ShopPurchasableClass || !GameState)
	{
		return;
	}

	if (Category == FShopRelatedWidget::MainWeapon || Category == FShopRelatedWidget::SecondaryWeapon)
	{
		for (int32 i = 0; i < GameState->GetPurchasableWeapons().Num(); i++)
		{
			AWeapon* Weapon = Cast<AWeapon>(GameState->GetPurchasableWeapons()[i]->GetDefaultObject());
			UShopPurchasable* Purchasable = CreateWidget<UShopPurchasable>(this, ShopPurchasableClass);
			Purchasable->SetPurchasableValues(this, Weapon->GetDisplayName(), Weapon->GetIcon(), Weapon->GetCost(), Category, i);
			Purchasables->AddChild(Purchasable);
		}
	}
	else if (Category == FShopRelatedWidget::Ability)
	{
		for (int32 i = 0; i < GameState->GetPurchasableWeapons2Temp().Num(); i++)
		{
			AWeapon* Weapon = Cast<AWeapon>(GameState->GetPurchasableWeapons2Temp()[i]->GetDefaultObject());
			UShopPurchasable* Purchasable = CreateWidget<UShopPurchasable>(this, ShopPurchasableClass);
			Purchasable->SetPurchasableValues(this, Weapon->GetDisplayName(), Weapon->GetIcon(), Weapon->GetCost(), Category, i);
			Purchasables->AddChild(Purchasable);
		}
	}
	Purchasables->SnapScrollBar(true);
}

void UShop::OnShopClose()
{
	Purchasables->RemoveAllChildren();
}

void UShop::OnPurchasableClicked(UShopPurchasable* PurchasableClicked)
{
	ABlasterPlayerController* Controller = Cast<ABlasterPlayerController>(GetOwningPlayer());
	if (!Controller)
	{
		return;
	}
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(Controller->GetCharacter());
	if (!Character)
	{
		return;
	}
	ABlasterPlayerState* PlayerState = Cast<ABlasterPlayerState>(Character->GetPlayerState());
	if (!PlayerState)
	{
		return;
	}

	uint32 Cost = PurchasableClicked->GetCost();
	uint32 Money = PlayerState->GetMoney();
	if (Cost > Money)
	{
		// Deny purchase
		PurchasableClicked->PlayDenyPurchaseAnimation();
		Controller->PlayNotEnoughMoneyAnimation();
		return;
	}
	// Accept purchase
	PurchasableClicked->PlayAcceptPurchaseAnimation();
	
	Character->ServerShopPurchase(PurchasableClicked->GetCategory(), PurchasableClicked->GetIndex());
}

