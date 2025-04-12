// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chat.generated.h"

class UBlasterScrollBox;
class UMultiLineEditableText;
class UEditableText;

/**
 * 
 */
UCLASS()
class BLASTER_API UChat : public UUserWidget
{
	GENERATED_BODY()
	

public:
	void ToggleChat();
	void ToggleChat(bool bShowChat);

	/**
	 * Bind widgets
	 */

	UPROPERTY(meta = (BindWidget))
	UBlasterScrollBox* MessagesScrollBox;

	UPROPERTY(meta = (BindWidget))
	UEditableText* MessageInputBox;

private:
	UFUNCTION()
	void OnMultiLineEditableTextCommittedEvent(const FText& Text, ETextCommit::Type CommitMethod);

	void SendMessage();


	bool bShown = false;
};
