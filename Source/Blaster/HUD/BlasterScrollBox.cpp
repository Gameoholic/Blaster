// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/Border.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/ListView.h"

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

	if (ScrollWheelEmpty)
	{
		ScrollWheelEmpty->SetPadding(FMargin(ScrollWheelSize, 0.0f, 0.0f, 0.0f));
		ScrollWheelEmptySlot = Cast<UVerticalBoxSlot>(ScrollWheelEmpty->Slot);
	}
	if (ScrollWheelFull)
	{
		ScrollWheelFull->SetPadding(FMargin(ScrollWheelSize, 0.0f, 0.0f, 0.0f));
		ScrollWheelFullSlot = Cast<UVerticalBoxSlot>(ScrollWheelFull->Slot);
	}

	if (bAlwaysShowScrollbar)
	{
		ScrollWheelFull->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ScrollWheelFull->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UBlasterScrollBox::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	//const FGeometry ScrollPanelGeometry = FindChildGeometry(MyGeometry, ItemsBox.ToSharedRef());
	////ScrollPanelGeometry.GetLocalSize()
	//const float ContentSize = GetScrollComponentFromVector(ScrollPanel->GetDesiredSize());
	// If internal children were updated and scroll box needs to be updated as a result
	if (bOnLastTickInternalChildrenUpdated == 1)
	{
		// Only update the scroll box if all newly created childrens' geometry is valid (usually not valid on the first tick of creation)
		bool bAllItemsBoxChildrenGeometryValid = true;
		//for (UWidget* ItemsBoxChild : ItemsBox->GetAllChildren())
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("UMM WAHT THE SIGMA %f VALID: %d"), ItemsBoxChild->GetCachedGeometry().GetAbsoluteSize(), ItemsBoxChild->GetCachedGeometry().GetAbsoluteSize().Y != 0.0f);
		//	if (ItemsBoxChild->GetCachedGeometry().GetAbsoluteSize().Y == 0.0f)
		//	{
		//		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("ONE OF THE CHILDREN INVALID, BREAKING"));
		//		bAllItemsBoxChildrenGeometryValid = false;
		//		break;
		//	}
		//}

		if (bAllItemsBoxChildrenGeometryValid)
		{
			UE_LOG(LogTemp, Warning, TEXT("ALL CHILDREN VALID"));
			UpdateScrollBox();
			bOnLastTickInternalChildrenUpdated = -1;
		}
	}
	if (bOnLastTickInternalChildrenUpdated == 0)
	{
		bOnLastTickInternalChildrenUpdated = 1;
	}
}

// TODO ADD ANIMATIONS JUST LIKE BLASTERT FILLABLE BAR TO SCROLL WHEEL


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

void UBlasterScrollBox::MoveScrollWheel(int32 Direction)
{
	ChildrenPosition += Direction * ScrollWheelChangeAmount;
	if (bOnLastTickInternalChildrenUpdated != -1)
	{
		// If children were JUST created, their geometry will be in invalid. Then we will update the scroll box on the next tick anyway, so don't update
		return;
	}
	UpdateScrollBox();
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
void UBlasterScrollBox::UpdateScrollBox()
{
	CalculatePositions();
	MoveChildren();
	UpdateScrollWheel();
}

void UBlasterScrollBox::UpdateChildren()
{
	//ListView->ClearListItems();
	ItemsBox->ClearChildren();
	if (bReverseOrder)
	{
		for (int32 i = InternalChildren.Num() - 1; i >= 0; i--)
		{
			ItemsBox->AddChild(InternalChildren[i]);
			//ListView->AddItem(InternalChildren[i]);
		}
	}
	else
	{
		for (int32 i = 0; i < InternalChildren.Num(); i++)
		{
			ItemsBox->AddChild(InternalChildren[i]);
			//ListView->AddItem(InternalChildren[i]);
		}
	}

	//for (UWidget* newchild : ItemsBox->GetAllChildren())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Needs prepass: %d"), newchild->TakeWidget()->NeedsPrepass());
	//}
}

void UBlasterScrollBox::CalculatePositions()
{


	FVector2D ThisDesiredSize = FVector2D::ZeroVector;
	for (int32 SlotIndex = 0; SlotIndex < ItemsBox->GetAllChildren().Num(); ++SlotIndex)
	{
		//const SScrollBox::FSlot& ThisSlot = Children[SlotIndex];
		if (ItemsBox->GetAllChildren()[SlotIndex]->GetVisibility() != ESlateVisibility::Collapsed)
		{
			const FVector2D ChildDesiredSize = ItemsBox->GetAllChildren()[SlotIndex]->GetDesiredSize();

			
			ThisDesiredSize.X = FMath::Max(ChildDesiredSize.X + Cast<UVerticalBoxSlot>(ItemsBox->GetAllChildren()[SlotIndex]->Slot)->Padding.GetTotalSpaceAlong<Orient_Horizontal>(), ThisDesiredSize.X);
			float test = ChildDesiredSize.Y + Cast<UVerticalBoxSlot>(ItemsBox->GetAllChildren()[SlotIndex]->Slot)->Padding.GetTotalSpaceAlong<Orient_Vertical>();
			ThisDesiredSize.Y += test;
			UE_LOG(LogTemp, Warning, TEXT("SEX Child sex: %f"), test);

		
			// add code for horizontal as well
		
		}
	}

	FVector2D::FReal ScrollPadding = true ? GetTickSpaceGeometry().GetLocalSize().Y : GetTickSpaceGeometry().GetLocalSize().X;
	FVector2D::FReal& SizeSideToPad = true ? ThisDesiredSize.Y : ThisDesiredSize.X;
	//SizeSideToPad += BackPadScrolling ? ScrollPadding : 0;
	//SizeSideToPad += FrontPadScrolling ? ScrollPadding : 0;


	UE_LOG(LogTemp, Warning, TEXT("SEX Child Size: X: %f Y: %f"), ThisDesiredSize.X, ThisDesiredSize.Y);

	return;


	return;
	for (UWidget* TestChild : ItemsBox->GetAllChildren())
	{




	








		
		 FVector2f POS = TestChild->GetCachedGeometry().GetAbsolutePosition();
		Cast<UTextBlock>(TestChild)->SetText(FText::FromString(FString::Printf(TEXT("test: %f AND %f"), POS.X, POS.Y)));
		UE_LOG(LogTemp, Warning, TEXT("Child Size: %f %f valid: %d"), 
			TestChild->GetCachedGeometry().GetAbsoluteSize().X, TestChild->GetCachedGeometry().GetAbsoluteSize().Y,
			TestChild->GetCachedWidget().IsValid());


		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, FString::Printf(TEXT("test: %f AND %f"), POS.X, POS.Y));
		//UE_LOG(LogTemp, Warning, TEXT("POSITION OF CURRENT: %f and %f"), POS.X, POS.Y);
	}
	//UE::Slate::FDeprecateVector2DResult PositionOfFurthestChild = ItemsBox->GetAllChildren()[ItemsBox->GetAllChildren().Num() - 1]->GetCachedGeometry().GetAbsolutePosition();
	//UE::Slate::FDeprecateVector2DResult PositionOfFurthestChild = ItemsBox->GetAllChildren()[ItemsBox->GetAllChildren().Num() - 1]->GetCachedGeometry().GetAbsolutePosition();

	//UE_LOG(LogTemp, Warning, TEXT("POSITION OF FURTHEST: %f and %f"), PositionOfFurthestChild.X, PositionOfFurthestChild.Y);
}



void UBlasterScrollBox::MoveChildren()
{
	for (UWidget* Child : ItemsBox->GetAllChildren())
	{
		Child->SetRenderTranslation(FVector2D(0.0f, ChildrenPosition));
	}
}

void UBlasterScrollBox::UpdateScrollWheel()
{

}



