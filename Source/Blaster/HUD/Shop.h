// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Shop.generated.h"

class UBlasterScrollBox;

/**
 * 
 */
UCLASS()
class BLASTER_API UShop : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class AWeapon>> PurchasableWeapons;

private:
	UPROPERTY(meta = (BindWidget))
	UBlasterScrollBox* Purchasables;


	UFUNCTION(BlueprintCallable, meta= (AllowPrivateAccess = "true"))
	FORCEINLINE UBlasterScrollBox* GetPurchasables() const { return Purchasables; }
};
