// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Shop.generated.h"

class UBlasterScrollBox;
class AWeapon;
class UShopPurchasable;

/**
 * 
 */
UCLASS()
class BLASTER_API UShop : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void DisplayPurchasables(FShopRelatedWidget Category);

private:
	UPROPERTY(meta = (BindWidget))
	UBlasterScrollBox* Purchasables;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UShopPurchasable> ShopPurchasableClass;
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AWeapon>> PurchasableWeapons;


	UFUNCTION(BlueprintCallable, meta= (AllowPrivateAccess = "true"))
	FORCEINLINE UBlasterScrollBox* GetPurchasables() const { return Purchasables; }
};
