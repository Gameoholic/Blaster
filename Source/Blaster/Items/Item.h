// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class BLASTER_API UItem : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Init(FString _Name, FSlateBrush _Icon);

protected:
	// Call to activate the item's perks
	void Activate();
	// Call to unactivate the item's perks (on death, on removal of item)
	void Deactivate();
private:
	UPROPERTY(EditDefaultsOnly)
	FString Name;
	UPROPERTY(EditDefaultsOnly)
	FSlateBrush Icon;
public:
	FORCEINLINE FString GetName() { return Name; }
	FORCEINLINE FSlateBrush GetIcon() { return Icon; }
};
