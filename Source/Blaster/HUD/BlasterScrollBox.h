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
	/**
	 * Construction parameters (modifies existing widgets)
	 */
	//
	UPROPERTY(EditDefaultsOnly)
	bool bTopToBottom = true;
	UPROPERTY(EditDefaultsOnly)
	float ScrollWheelSize;
	
	/**
	 * Other parameters
	 */
	//
	UPROPERTY(EditDefaultsOnly)
	bool bAlwaysShowScrollbar = false;
	UPROPERTY(EditDefaultsOnly)
	bool bReverseOrder = false;

	/**
	 * Bind widgets
	 */
	// 
	UPROPERTY(meta = (BindWidget))
	UBorder* ScrollWheel;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ItemsBox;

	/** 
	 * Methods to modify scrollbox after construction
	 */

	// 
	UFUNCTION(BlueprintCallable)
	void AddChild(UWidget* WidgetToAdd);

	UFUNCTION(BlueprintCallable)
	void AddChildren(TArray<UWidget*> WidgetsToAdd);

private:
	// Because Unreal Engine is stupid, we have to keep track of all children in the original, unreversed order if we want to reverse it and be able to add new children and reverse them as well. We recreate the array and reverse it every time a new child is added. No other way is possible currently.
	TArray<UWidget*> OriginalChildren;

	// Adds child to internal array without actually adding it to ItemsBox
	void InternalAddChild(UWidget* WidgetToAdd);
	// Adds children to ItemsBox and rearranges them based on whether order is reversed or not
	void DisplayChildren();


};
