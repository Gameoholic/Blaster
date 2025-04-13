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
class UBorder;

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
	// Called from blasterplayercontroller. Adds the messages without trigerring an opacity transition or displaying
	void AddMessagesSilently(TArray<FName> Messages);

	/**
	 * Bind widgets
	 */

	UPROPERTY(meta = (BindWidget))
	UBlasterScrollBox* MessagesScrollBox;

	UPROPERTY(meta = (BindWidget))
	UEditableText* MessageInputText;

	UPROPERTY(meta = (BindWidget))
	UBorder* MessageInputBox;

	// How long should the grace period for new messages be before they start disappearing
	UPROPERTY(EditAnywhere, Category = "New Messages")
	float NewMessagesGracePeriodDuration = 3.0f;
	// How long should the opacity transition for new messages last
	UPROPERTY(EditAnywhere, Category = "New Messages")
	float NewMessagesTransitionDuration = 1.0f;
	// Exponential for easing of opacity transition
	UPROPERTY(EditAnywhere, Category = "New Messages")
	float NewMessagesTransitionExponential = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Messages")
	FSlateFontInfo MessageFont;
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
	// If set to silent, will not display the message immediately and trigger an opacity transition
	void DisplayMessage(FName Message, bool bSilent);

	// New messages will have a timer of how long they've existed (opacity changes)
	TMap<UTextBlock*, float> NewMessages;
};
