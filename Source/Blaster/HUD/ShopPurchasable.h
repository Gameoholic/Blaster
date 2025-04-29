// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopPurchasable.generated.h"

class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class BLASTER_API UShopPurchasable : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetPurchasableValues(FString _Name, FSlateBrush _Icon, int32 _Cost);

private:
	int32 Cost = -1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PurchasableName;

	UPROPERTY(meta = (BindWidget))
	UImage* PurchasableIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PurchasableCost;

};
