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
#include "Components/Border.h"
#include "GameFramework/PlayerState.h"

void UChat::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	Character = Cast<ABlasterCharacter>(GetOwningPlayer()->GetCharacter());

	// Chat should be hidden by default: (don't call ToggleChat(false) it has a lot of extra stuff we don't need and will crash if called this early)
	MessageInputBox->SetVisibility(ESlateVisibility::Hidden);
	MessagesScrollBox->ForceHideScrollBar(true);
	// Messages might already exist, hide them too
	for (UWidget* Message : MessagesScrollBox->GetChildren())
	{
		Cast<UTextBlock>(Message)->SetOpacity(0.0f);
	}
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

	MessagesScrollBox->SnapScrollBar(true);
	if (bShown)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeData.SetWidgetToFocus(MessageInputText->TakeWidget());
		GetOwningPlayer()->SetInputMode(InputModeData);
		GetOwningPlayer()->SetShowMouseCursor(true);
		MessageInputText->OnTextCommitted.AddDynamic(this, &UChat::OnMultiLineEditableTextCommittedEvent);

		// Show all messages & inputbox
		for (UWidget* Message : MessagesScrollBox->GetChildren())
		{
			Cast<UTextBlock>(Message)->SetOpacity(1.0f);
		}
		MessageInputBox->SetVisibility(ESlateVisibility::Visible);

		MessagesScrollBox->ForceHideScrollBar(false);
		MessagesScrollBox->RequestUpdateScrollBox(true);
	}
	else
	{
		FInputModeGameOnly InputModeData;
		GetOwningPlayer()->SetInputMode(InputModeData);
		GetOwningPlayer()->SetShowMouseCursor(false);

		MessageInputText->OnTextCommitted.RemoveAll(this);

		// Hide all messages & inputbox
		for (UWidget* Message : MessagesScrollBox->GetChildren())
		{
			Cast<UTextBlock>(Message)->SetOpacity(0.0f);
		}
		MessageInputBox->SetVisibility(ESlateVisibility::Hidden);

		MessagesScrollBox->ForceHideScrollBar(true);
		MessagesScrollBox->RequestUpdateScrollBox(true);
	}
}



void UChat::OnMultiLineEditableTextCommittedEvent(const FText& Text, ETextCommit::Type CommitMethod)
{
	// If event happened after chat was already closed, ignore it
	if (!bShown)
	{
		return;
	}

	if (CommitMethod == ETextCommit::Type::OnEnter && Text.ToString().Len() > 0)
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

	DisplayMessage(Message, false);
	if (Character)
	{
		Character->OnChatMessageReceived(Message);
	}
}

void UChat::AddMessagesSilently(TArray<FName> Messages)
{
	for (FName Message : Messages)
	{
		DisplayMessage(Message, true);
	}
}

void UChat::SendMessage()
{
	FName Message = FName(MessageInputText->GetText().ToString());
	MessageInputText->SetText(FText());
	if (Character != nullptr)
	{
		Character->ServerSendPlayerChatMessage(Message, FName(Character->GetPlayerState()->GetPlayerName())); // This will trigger ReceiveMessage()
	}
}

void UChat::DisplayMessage(FName Message, bool bSilent)
{
	UBlasterTextBlock* TextBlock = WidgetTree->ConstructWidget<UBlasterTextBlock>(UBlasterTextBlock::StaticClass());
	TextBlock->SetText(FText::FromName(Message));
	TextBlock->SetAutoWrapText(true);
	TextBlock->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
	TextBlock->SetFont(MessageFont);

	MessagesScrollBox->AddChild(TextBlock);
	// The child that's now in the scrollbox is different from the one we added

	if (!bSilent)
	{
		NewMessages.Add(TextBlock, 0.0f);
	}
}

