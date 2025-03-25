// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlasterScrollBox.generated.h"

class UWidget;
class UBorder;
class UVerticalBox;

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
	// ScrollBox construction parameters
	UPROPERTY(EditDefaultsOnly)
	bool bTopToBottom = true;
	UPROPERTY(EditDefaultsOnly)
	float ScrollWheelSize;
	
	// Other parameters
	UPROPERTY(EditDefaultsOnly)
	bool bAlwaysShowScrollbar = false;

	// Bind widgets
	UPROPERTY(meta = (BindWidget))
	UBorder* ScrollWheel;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ItemsBox;

	// Methods to modify scrollbox after construction
	UFUNCTION(BlueprintCallable)
	void AddChild(UWidget* WidgetToAdd);

};
