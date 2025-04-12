// Fill out your copyright notice in the Description page of Project Settings.


#include "Chat.h"
#include "Components/MultiLineEditableText.h"
#include "Components/EditableText.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterScrollBox.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Blaster/HUD/BlasterTextBlock.h"

void UChat::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Character = Cast<ABlasterCharacter>(GetOwningPlayer()->GetCharacter());
}

void UChat::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Adjust opacity for new messages
	for (TMap<UTextBlock*, float>::TIterator Message = NewMessages.CreateIterator(); Message; ++Message)
	{
		*(&Message.Value()) += InDeltaTime;
		// Only change opacity if chat is hidden
		if (!bShown)
		{
			Message.Key()->SetOpacity(FMath::InterpEaseInOut(
				1.0f,
				0.0f,
				FMath::Max(Message.Value() - NewMessagesGracePeriodDuration, 0.0f) / NewMessagesTransitionDuration,
				NewMessagesTransitionExponential
			));
		}
		// Remove from list if completely disappeared
		if (Message.Value() - NewMessagesGracePeriodDuration >= NewMessagesTransitionDuration)
		{
			Message.RemoveCurrent();
		}
	}
}

void UChat::ToggleChat()
{
	ToggleChat(!bShown);
}

void UChat::ToggleChat(bool bShowChat)
{
	bShown = bShowChat;
	if (bShown)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeData.SetWidgetToFocus(MessageInputBox->TakeWidget());
		GetOwningPlayer()->SetInputMode(InputModeData);
		GetOwningPlayer()->SetShowMouseCursor(true);
		MessageInputBox->OnTextCommitted.AddDynamic(this, &UChat::OnMultiLineEditableTextCommittedEvent);

		// Show all messages
		for (TMap<UTextBlock*, float>::TIterator Message = NewMessages.CreateIterator(); Message; ++Message)
		{
			Message.Key()->SetOpacity(1.0f);
		}
	}
	else
	{
		FInputModeGameOnly InputModeData;
		GetOwningPlayer()->SetInputMode(InputModeData);
		GetOwningPlayer()->SetShowMouseCursor(false);

		MessageInputBox->OnTextCommitted.RemoveAll(this);

		// Hide all messages
		for (TMap<UTextBlock*, float>::TIterator Message = NewMessages.CreateIterator(); Message; ++Message)
		{
			Message.Key()->SetOpacity(0.0f);
		}
	}
}



void UChat::OnMultiLineEditableTextCommittedEvent(const FText& Text, ETextCommit::Type CommitMethod)
{
	// If event happened after chat was already closed, ignore it
	if (!bShown)
	{
		return;
	}

	if (CommitMethod == ETextCommit::Type::OnEnter)
	{
		SendMessage();
	}
	ToggleChat(false);
}

void UChat::ReceiveMessage(FName Message)
{
	if (!WidgetTree)
	{
		return;
	}

	DisplayMessage(Message);
}

void UChat::SendMessage()
{
	FName Message = FName(MessageInputBox->GetText().ToString());
	MessageInputBox->SetText(FText());
	if (Character != nullptr)
	{
		Character->ServerSendPlayerChatMessage(Message); // This will trigger ReceiveMessage()
	}
}

void UChat::DisplayMessage(FName Message)
{
	UBlasterTextBlock* TextBlock = WidgetTree->ConstructWidget<UBlasterTextBlock>(UBlasterTextBlock::StaticClass());
	TextBlock->SetText(FText::FromName(Message));
	TextBlock->SetAutoWrapText(true);
	TextBlock->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);

	MessagesScrollBox->AddChild(TextBlock);
	// The child that's now in the scrollbox is different from the one we added

	NewMessages.Add(TextBlock, 0.0f);
}

