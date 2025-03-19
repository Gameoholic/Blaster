// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UItem : public UObject
{
	GENERATED_BODY()
private:
	UPROPERTY(EditDefaultsOnly)
	FString Name;
	UPROPERTY(EditDefaultsOnly)
	FSlateBrush Icon;
public:
	FORCEINLINE FString GetName() { return Name; }
	FORCEINLINE FSlateBrush GetIcon() { return Icon; }
};
