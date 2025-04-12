// Fill out your copyright notice in the Description page of Project Settings.


#include "Chat.h"
#include "Components/MultiLineEditableText.h"
#include "Components/EditableText.h"


void UChat::ToggleChat()
{
	ToggleChat(!bShown);
}

void UChat::ToggleChat(bool bShowChat)
{
	UE_LOG(LogTemp, Warning, TEXT("Toggling chat."));
	bShown = bShowChat;
	if (bShown)
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeData.SetWidgetToFocus(MessageInputBox->TakeWidget());
		GetOwningPlayer()->SetInputMode(InputModeData);
		GetOwningPlayer()->SetShowMouseCursor(true);


		MessageInputBox->OnTextCommitted.AddDynamic(this, &UChat::OnMultiLineEditableTextCommittedEvent);
	}
	else
	{
		FInputModeGameOnly InputModeData;
		GetOwningPlayer()->SetInputMode(InputModeData);
		GetOwningPlayer()->SetShowMouseCursor(false);

		MessageInputBox->OnTextCommitted.RemoveAll(this);
	}
}

void UChat::SendMessage()
{
	UE_LOG(LogTemp, Warning, TEXT("Sending message."));
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
	else
	{
		ToggleChat(false);
	}


	//enum Type : int
	//{
	//	/** Losing focus or similar event caused implicit commit */
	//	Default, // 0
	//	/** User committed via the enter key */
	//	OnEnter, // 1
	//	/** User committed via tabbing away or moving focus explicitly away */
	//	OnUserMovedFocus, // 2
	//	/** Keyboard focus was explicitly cleared via the escape key or other similar action */
	//	OnCleared // 3
	//};
}