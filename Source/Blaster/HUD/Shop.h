// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Shop.generated.h"

class UBlasterScrollBox;
class AWeapon;
class UShopPurchasable;
class ABlasterGameState;

/**
 * 
 */
UCLASS()
class BLASTER_API UShop : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void DisplayPurchasables(FShopRelatedWidget Category);
	void OnShopClose();
	void OnPurchasableClicked(UShopPurchasable* PurchasableClicked);
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	ABlasterGameState* GameState;

	UPROPERTY(meta = (BindWidget))
	UBlasterScrollBox* Purchasables;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UShopPurchasable> ShopPurchasableClass;

	UFUNCTION(BlueprintCallable, meta= (AllowPrivateAccess = "true"))
	FORCEINLINE UBlasterScrollBox* GetPurchasables() const { return Purchasables; }
};
