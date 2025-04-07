// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlasterScrollBox.generated.h"

class UWidget;
class UBorder;
class UVerticalBox;
class UVerticalBoxSlot;
class UListView;

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterScrollBox : public UUserWidget
{
	GENERATED_BODY()


	
protected:
	virtual void NativePreConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


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
	UPROPERTY(EditDefaultsOnly)
	float ScrollWheelChangeAmount = 20.0f;
	/** 
	 * Methods to modify scrollbox after construction
	 */

	// 
	UFUNCTION(BlueprintCallable)
	void AddChild(UWidget* WidgetToAdd);

	UFUNCTION(BlueprintCallable)
	void AddChildren(TArray<UWidget*> WidgetsToAdd);

	UFUNCTION(BlueprintCallable)
	void MoveScrollWheel(int32 Direction);

private:
	// Because Unreal Engine is stupid, we have to keep track of all children in the original, unreversed order if we want to reverse it and be able to add new children and reverse them as well. We recreate the array and reverse it every time a new child is added. No other way is possible currently.
	TArray<UWidget*> InternalChildren;

	int32 bOnLastTickInternalChildrenUpdated = -1;

	// Adds child to internal array without actually adding it to ItemsBox
	void InternalAddChild(UWidget* WidgetToAdd);
	// Updates the entire scroll box's hud based on children, after their geometry has been generated
	void UpdateScrollBox();
	// Adds children to ItemsBox and rearranges them based on whether order is reversed or not
	void UpdateChildren();
	// When internal children array is changed
	void OnInternalChildrenChanged();

	void CalculatePositions();
	void MoveChildren();
	void UpdateScrollWheel();

	float ChildrenPosition;

	/**
	 * Bind widgets
	 */
	// 
	// The empty/invisible part of the scroll wheel
	UPROPERTY(meta = (BindWidget))
	UBorder* ScrollWheelEmpty;

	// The full/colored part of the scroll wheel
	UPROPERTY(meta = (BindWidget))
	UBorder* ScrollWheelFull;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ItemsBox;

	//UPROPERTY(meta = (BindWidget))
	//UListView* ListView;

	UVerticalBoxSlot* ScrollWheelFullSlot = nullptr;
	UVerticalBoxSlot* ScrollWheelEmptySlot = nullptr;





};
