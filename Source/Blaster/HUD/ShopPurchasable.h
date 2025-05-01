// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "ShopPurchasable.generated.h"

class UTextBlock;
class UImage;
class UBorder;
class UButton;
class UShop;

/**
 * 
 */
UCLASS()
class BLASTER_API UShopPurchasable : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UFUNCTION(BlueprintCallable)
	void SetPurchasableValues(UShop* _Shop, FString _Name, FSlateBrush _Icon, int32 _Cost, FShopRelatedWidget _Category, int32 _Index);
	void PlayDenyPurchaseAnimation();
	void PlayAcceptPurchaseAnimation();
	FORCEINLINE int32 GetCost() { return Cost; }
	FORCEINLINE FShopRelatedWidget GetCategory() { return Category; }
	FORCEINLINE int32 GetIndex() { return Index; }

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	UShop* Shop;

	int32 Index = -1;
	FShopRelatedWidget Category = FShopRelatedWidget::None;
	int32 Cost = -1;

	UPROPERTY(meta = (BindWidget))
	UButton* Button;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PurchasableName;

	UPROPERTY(meta = (BindWidget))
	UImage* PurchasableIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PurchasableCost;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* DenyAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* AcceptAnimation;

	UFUNCTION()
	void OnClicked();


};
