// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlasterScrollBox.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterScrollBox : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativePreConstruct() override;

public:
	UPROPERTY(EditDefaultsOnly)
	bool bTopToBottom = true;

	UPROPERTY(meta = (BindWidget))
	class UBorder* ScrollWheel;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ItemsBox;
};
