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

	void SetScrollWheelPartSize(UVerticalBoxSlot* ScrollWheelPart, float Size);

	float GetDPIScale();

	float ChildrenPosition = 0.0f; // Children position according to top-to-bottom/bottom-to-top of items box (increasing it means we're "scrolling" up/down the items box, decreasing returns it to original order), in slate units.

	// The following variables are updated on UpdateScrollBox()
	float UnrenderedItemsAboveSize = 0.0f; // The size of all items that are not rendered, above the viewport. In slate units.
	float RenderedItemsSize = 0.0f; // The size of all items that are rendered in the viewport. In slate units. (size of the visible rendered items box)
	float UnrenderedItemsBelowSize = 0.0f; // The size of all items that are not rendered, below the viewport. In slate units.
	float ItemsBoxTotalSize = 0.0f; // Total DESIRED size of the items box, including all items inside it, in slate units.


	/**
	 * Bind widgets
	 */
	// 
	// The empty/invisible TOP part of the scroll wheel
	UPROPERTY(meta = (BindWidget))
	UBorder* ScrollWheelTop;

	// The full/colored part of the scroll wheel
	UPROPERTY(meta = (BindWidget))
	UBorder* ScrollWheelMiddle;

	// The empty/invisible BOTTOM part of the scroll wheel
	UPROPERTY(meta = (BindWidget))
	UBorder* ScrollWheelBottom;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ItemsBox;

	//UPROPERTY(meta = (BindWidget))
	//UListView* ListView;

	UVerticalBoxSlot* ScrollWheelTopSlot = nullptr;
	UVerticalBoxSlot* ScrollWheelMiddleSlot = nullptr;
	UVerticalBoxSlot* ScrollWheelBottomSlot = nullptr;

	




};
