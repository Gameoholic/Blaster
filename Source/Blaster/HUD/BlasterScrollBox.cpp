// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/Border.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"
#include "Runtime/Engine/Classes/Engine/UserInterfaceSettings.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void UBlasterScrollBox::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Change all widgets according to parameters
	if (ItemsBox)
	{
		if (bTopToBottom)
		{
			Cast<UHorizontalBoxSlot>(ItemsBox->Slot)->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
			ItemsBox->SetRenderTransformAngle(0); // So the order will be top to bottom, with bottom ones clipping out
		}
		else
		{
			Cast<UHorizontalBoxSlot>(ItemsBox->Slot)->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
			ItemsBox->SetRenderTransformAngle(180); // So the order will be bottom to top, with top ones clipping out
		}
	}

	if (ScrollWheelTop)
	{
		ScrollWheelTop->SetPadding(FMargin(ScrollWheelSize, 0.0f, 0.0f, 0.0f));
		ScrollWheelTopSlot = Cast<UVerticalBoxSlot>(ScrollWheelTop->Slot);
	}
	if (ScrollWheelMiddle)
	{
		ScrollWheelMiddle->SetPadding(FMargin(ScrollWheelSize, 0.0f, 0.0f, 0.0f));
		ScrollWheelMiddleSlot = Cast<UVerticalBoxSlot>(ScrollWheelMiddle->Slot);
	}
	if (ScrollWheelBottom)
	{
		ScrollWheelBottom->SetPadding(FMargin(ScrollWheelSize, 0.0f, 0.0f, 0.0f));
		ScrollWheelBottomSlot = Cast<UVerticalBoxSlot>(ScrollWheelBottom->Slot);
	}
}

void UBlasterScrollBox::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetOwningPlayer());
	if (!PlayerController)
	{
		return;
	}
	Character = Cast<ABlasterCharacter>(PlayerController->GetCharacter());
	if (!Character)
	{
		return;
	}
	Character->FocusedScrollBox = this;
}

void UBlasterScrollBox::NativeDestruct()
{
	if (!Character)
	{
		return;
	}
	Character->FocusedScrollBox = nullptr;
}

void UBlasterScrollBox::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bOnLastTickInternalChildrenUpdated == -1 && SmoothScrollingChangeProgress < 1.0f)
	{
		SmoothScroll(InDeltaTime);
		UpdateScrollBox();
		return;
	}

	// If internal children were updated, a tick has passed and scroll box needs to be updated as a result
	if (bOnLastTickInternalChildrenUpdated == 1)
	{
		UpdateScrollBox();
		bOnLastTickInternalChildrenUpdated = -1;
	}
	if (bOnLastTickInternalChildrenUpdated == 0)
	{
		bOnLastTickInternalChildrenUpdated = 1;
	}
}

void UBlasterScrollBox::AddChild(UWidget* WidgetToAdd)
{
	InternalAddChild(WidgetToAdd);
	OnInternalChildrenChanged();
}

void UBlasterScrollBox::AddChildren(TArray<UWidget*> WidgetsToAdd)
{
	for (UWidget* Widget : WidgetsToAdd)
	{
		InternalAddChild(Widget);
	}
	OnInternalChildrenChanged();
}


void UBlasterScrollBox::HandleMouseWheelScroll(float MouseWheelDirection)
{
	if (!bEnableMouseScrollWheel)
	{
		return;
	}
	float ScrollBarMoveDirection = MouseWheelDirection;
	ScrollBarMoveDirection = bClampMouseScrollWheelValues ? FMath::Clamp(ScrollBarMoveDirection, -1.0f, 1.0f) : ScrollBarMoveDirection;
	ScrollBarMoveDirection = bReverseMouseScrollWheelDirection ? -ScrollBarMoveDirection : ScrollBarMoveDirection;
	MoveScrollBar(ScrollBarMoveDirection);
}

void UBlasterScrollBox::MoveScrollBar(float Direction)
{
	if (bOverrideScrollWheelChange)
	{
		SmoothScrollingTargetChildrenPosition = ChildrenPosition + Direction * ScrollWheelChangeAmount / GetDPIScale(); // Make scroll wheel change amount universal regardless of screen size by dividing by DPI Scale
	}
	else
	{
		SmoothScrollingTargetChildrenPosition += Direction * ScrollWheelChangeAmount / GetDPIScale(); // Make scroll wheel change amount universal regardless of screen size by dividing by DPI Scale
	}
	SmoothScrollingChangeProgress = 0.0f;
}

void UBlasterScrollBox::InternalAddChild(UWidget* WidgetToAdd)
{
	bTopToBottom ? WidgetToAdd->SetRenderTransformAngle(0) : WidgetToAdd->SetRenderTransformAngle(180); // Needs to match angle of ItemsBox
	InternalChildren.Add(WidgetToAdd);
}

void UBlasterScrollBox::OnInternalChildrenChanged()
{
	UpdateChildren();
	// On next tick, the childrens' new positions will be calculated, children moved, scroll wheel updated, etc.
	// We can't do it on the same tick immediately because the geometry for the children isn't generated yet after UpdateChildren()
	bOnLastTickInternalChildrenUpdated = 0;
}

void UBlasterScrollBox::UpdateChildren()
{
	ItemsBox->ClearChildren();
	if (bReverseOrder)
	{
		for (int32 i = InternalChildren.Num() - 1; i >= 0; i--)
		{
			ItemsBox->AddChild(InternalChildren[i]);
		}
	}
	else
	{
		for (int32 i = 0; i < InternalChildren.Num(); i++)
		{
			ItemsBox->AddChild(InternalChildren[i]);
		}
	}
}

void UBlasterScrollBox::UpdateScrollBox()
{
	CalculateItemSizes();
	MoveChildren();
	UpdateScrollWheel();
}

void UBlasterScrollBox::CalculateItemSizes()
{
	// Calculate DPI scale for pixels->slate units conversion

	// Calculate all sizes
	// If items box top to bottom:
	ItemsBoxTotalSize = ItemsBox->GetDesiredSize().Y;
	UnrenderedItemsAboveSize = ChildrenPosition;
	RenderedItemsSize = ItemsBox->GetCachedGeometry().GetAbsoluteSize().Y / GetDPIScale(); // Size needs to be converted to slate units.
	RenderedItemsSize = bTopToBottom ? RenderedItemsSize : RenderedItemsSize * -1; // If bottom to top, render transform angle is set to 180 at bottom to top, so cached geometry size will return negative 1.
	UnrenderedItemsBelowSize = ItemsBoxTotalSize - RenderedItemsSize - ChildrenPosition;

	// If bottom to top:
	if (!bTopToBottom)
	{
		// Above and below will now be flipped, so we have to switch them:
		float AboveSize = UnrenderedItemsAboveSize; // temp variable for switching variables
		UnrenderedItemsAboveSize = UnrenderedItemsBelowSize;
		UnrenderedItemsBelowSize = AboveSize;
	}

	// Show/hide scrollbar if all items fit
	if (!bAlwaysShowScrollbar && RenderedItemsSize == ItemsBoxTotalSize)
	{
		ScrollWheelMiddle->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		ScrollWheelMiddle->SetVisibility(ESlateVisibility::Visible);
	}

	// Constrain children position if beyond bounds and recalculate positions if was clamped
	if (ChildrenPosition != FMath::Clamp(ChildrenPosition, 0.0f, ItemsBoxTotalSize - RenderedItemsSize))
	{
		ChildrenPosition = FMath::Clamp(ChildrenPosition, 0.0f, ItemsBoxTotalSize - RenderedItemsSize);
		SmoothScrollingTargetChildrenPosition = ChildrenPosition;
		SmoothScrollingChangeProgress = 1.0f;
		CalculateItemSizes();
	}
}

void UBlasterScrollBox::MoveChildren()
{
	for (UWidget* Child : ItemsBox->GetAllChildren())
	{
		Child->SetRenderTranslation(FVector2D(0.0f, -ChildrenPosition)); // Children position is actually opposite of UI render position
	}
}

void UBlasterScrollBox::UpdateScrollWheel()
{
	//UE_LOG(LogTemp, Warning, TEXT("Smooth Scrolling: %f"), SmoothScrollingCurrentPercentage);
	float ScrollWheelTopSize = UnrenderedItemsAboveSize / ItemsBoxTotalSize; // Percentage of the items box total size that's ABOVE the rendered area
	float ScrollWheelMiddleSize = RenderedItemsSize / ItemsBoxTotalSize; // Percentage of the items box total size that's currently rendered in the viewport (in the middle)
	float ScrollWheelBottomSize = UnrenderedItemsBelowSize / ItemsBoxTotalSize; // Percentage of the items box total size that's BELOW the rendered area

	SetScrollWheelPartSize(ScrollWheelTopSlot, ScrollWheelTopSize);
	SetScrollWheelPartSize(ScrollWheelMiddleSlot, ScrollWheelMiddleSize);
	SetScrollWheelPartSize(ScrollWheelBottomSlot, ScrollWheelBottomSize);
}

void UBlasterScrollBox::SetScrollWheelPartSize(UVerticalBoxSlot* ScrollWheelPart, float Size)
{
	FSlateChildSize SlotSize = FSlateChildSize(ESlateSizeRule::Fill);
	SlotSize.Value = Size;
	ScrollWheelPart->SetSize(SlotSize);
}

float UBlasterScrollBox::GetDPIScale()
{
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	int32 ViewportSizeX = FGenericPlatformMath::FloorToInt(ViewportSize.X); // There is some rounding but the effect on location accuracy is negligible
	int32 ViewportSizeY = FGenericPlatformMath::FloorToInt(ViewportSize.Y);
	return GetDefault<UUserInterfaceSettings>(UUserInterfaceSettings::StaticClass())->GetDPIScaleBasedOnSize(FIntPoint(ViewportSizeX, ViewportSizeY));
}

void UBlasterScrollBox::SmoothScroll(float DeltaTime)
{
	SmoothScrollingChangeProgress += DeltaTime / SmoothScrollingChangeDuration;
	SmoothScrollingChangeProgress = FMath::Clamp(SmoothScrollingChangeProgress, 0.0f, 1.0f);

	ChildrenPosition = FMath::InterpEaseInOut(
		ChildrenPosition,
		SmoothScrollingTargetChildrenPosition,
		SmoothScrollingChangeProgress,
		SmoothScrollingChangeExponential
	);
	UpdateScrollBox();
}



