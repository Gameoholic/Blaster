// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlasterScrollBox.generated.h"

class UWidget;
class UBorder;
class UVerticalBox;
class UHorizontalBox;
class UVerticalBoxSlot;
class UListView;
class ABlasterCharacter;

/**
 * IMPORTANT: When this widget is focused, you should call Init() otherwise certain functions won't work
 */
UCLASS()
class BLASTER_API UBlasterScrollBox : public UUserWidget
{
	GENERATED_BODY()


	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UPROPERTY(EditAnywhere, Category = "Children")
	bool bTopToBottom = true;
	// Whether to reverse the order of the children
	UPROPERTY(EditAnywhere, Category = "Children")
	bool bReverseOrder = false;
	// Whether to always show scrollbar even if all children fit in the viewport
	UPROPERTY(EditAnywhere, Category = "Scroll Bar")
	bool bAlwaysShowScrollbar = false;
	UPROPERTY(EditAnywhere, Category = "Scroll Bar")
	float ScrollWheelSize = 50.0f;
	// By how much to move the scroll wheel
	UPROPERTY(EditAnywhere, Category = "Scroll Bar")
	float ScrollWheelChangeAmount = 20.0f;
	// If true, each time the scroll wheel is requested to change, will ignore the previous change instead of stacking. Useful for buttons, not useful for mouse scroll wheel input.
	UPROPERTY(EditAnywhere, Category = "Scroll Bar")
	bool bOverrideScrollWheelChange = false;
	UPROPERTY(EditAnywhere, Category = "Scroll Bar")
	FSlateBrush ScrollBarAppearance = FSlateBrush::FSlateBrush();
	UPROPERTY(EditAnywhere, Category = "Scroll Bar")
	FMargin ScrollBarPadding = FMargin::FMargin(0.0f, 0.0f, 0.0f, 0.0f);


	// Whether to ignore the mouse scroll wheel input or not
	UPROPERTY(EditAnywhere, Category = "Scroll Wheel")
	bool bEnableMouseScrollWheel = true;
	// Whether to clamp scroll wheel to 1/-1 (can go to 2/-2 if scroll wheel is fast)
	UPROPERTY(EditAnywhere, Category = "Scroll Wheel")
	bool bClampMouseScrollWheelValues = false;
	UPROPERTY(EditAnywhere, Category = "Scroll Wheel")
	bool bReverseMouseScrollWheelDirection = false;

	// How long should the smooth scrolling transition last. Set to 0 to disable smooth scrolling
	UPROPERTY(EditAnywhere, Category = "Smooth Scrolling")
	float SmoothScrollingChangeDuration = 0.0f;
	// Exponential for easing of smooth scrolling
	UPROPERTY(EditAnywhere, Category = "Smooth Scrolling")
	float SmoothScrollingChangeExponential = 1.0f;

	// Called by BlasterCharacter after focus has been requested by this scrollbox, and has been granted. DO NOT assume focus was granted just because scroll box was hovered, only rely on this method.
	void OnFocus();
	// Called by BlasterCharacter after focus has been removed for this scrollbox.
	void OnUnfocus();
	// Called by BlasterCharacter if this scrollbox is focused and mouse scroll wheel had input
	void HandleMouseWheelScroll(float MouseWheelDirection);

	/** 
	 * Methods to modify scrollbox post construction
	 */

	UFUNCTION(BlueprintCallable)
	void AddChild(UWidget* WidgetToAdd);

	UFUNCTION(BlueprintCallable)
	void AddChildren(TArray<UWidget*> WidgetsToAdd);

	UFUNCTION(BlueprintCallable)
	void RemoveChildAt(int32 Index);

	UFUNCTION(BlueprintCallable)
	void RemoveAllChildren();

	UFUNCTION(BlueprintCallable)
	TArray<UWidget*> GetChildren();

	// Direction input can be given a value other than 1 or -1 to amplify the speed
	UFUNCTION(BlueprintCallable)
	void MoveScrollBar(float Direction);

	// Will snap scroll bottom to the bottom most or top most position immediately
	UFUNCTION(BlueprintCallable)
	void SnapScrollBar(bool bBottom);

	// Will update the entire scroll box (the childrens' new positions will be calculated, children moved, scroll wheel updated, etc.) immediately, unless set to not immediate. In that case, it will happen on the next tick.
	UFUNCTION(BlueprintCallable)
	void RequestUpdateScrollBox(bool bImmediate);

	// If forced to hide scroll bar, will not show it at all even if needed
	UFUNCTION(BlueprintCallable)
	void ForceHideScrollBar(bool bHide);

private:
	ABlasterCharacter* Character;

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

	void CalculateItemSizes();
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

	// Smooth scrolling
	float SmoothScrollingTargetChildrenPosition = 0.0f;
	// Between 0-1 how much of the transition have we done
	float SmoothScrollingChangeProgress = 0.0f;
	// This will be called per tick
	void SmoothScroll(float DeltaTime);

	FVector2D LastTickViewportSize = FVector2D::ZeroVector;
	/**
	 * Bind widgets
	 */
	
	// Root widget
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* BlasterScrollBox;
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

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ScrollWheelBox;

	UVerticalBoxSlot* ScrollWheelTopSlot = nullptr;
	UVerticalBoxSlot* ScrollWheelMiddleSlot = nullptr;
	UVerticalBoxSlot* ScrollWheelBottomSlot = nullptr;

	bool bForceHideScrollBar = false;
};

