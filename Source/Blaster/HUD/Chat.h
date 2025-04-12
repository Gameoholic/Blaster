// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chat.generated.h"

class UBlasterScrollBox;
class UMultiLineEditableText;
class UEditableText;
class ABlasterCharacter;
class UTextBlock;

/**
 * 
 */
UCLASS()
class BLASTER_API UChat : public UUserWidget
{
	GENERATED_BODY()
	

public:
	// Called from blasterplayer
	void ToggleChat();
	// Called from blasterplayer
	void ReceiveMessage(FName Message);

	/**
	 * Bind widgets
	 */

	UPROPERTY(meta = (BindWidget))
	UBlasterScrollBox* MessagesScrollBox;

	UPROPERTY(meta = (BindWidget))
	UEditableText* MessageInputBox;

	// How long should the opacity transition for new messages last.
	UPROPERTY(EditAnywhere, Category = "New Messages")
	float NewMessagesDuration = 5.0f;
	// Exponential for easing of opacity transition
	UPROPERTY(EditAnywhere, Category = "New Messages")
	float NewMessagesTransitionExponential = 1.0f;
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	void ToggleChat(bool bShowChat);
	ABlasterCharacter* Character;

	bool bShown = false;

	UFUNCTION()
	void OnMultiLineEditableTextCommittedEvent(const FText& Text, ETextCommit::Type CommitMethod);

	void SendMessage();
	void DisplayMessage(FName Message);

	// New messages will have a timer of how long they've existed (opacity changes)
	TMap<UTextBlock*, float> NewMessages;
};
