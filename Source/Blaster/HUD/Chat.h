// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Chat.generated.h"

class UBlasterScrollBox;

/**
 * 
 */
UCLASS()
class BLASTER_API UChat : public UUserWidget
{
	GENERATED_BODY()
	

public:
	/**
	 * Bind widgets
	 */

	UPROPERTY(meta = (BindWidget))
	UBlasterScrollBox* MessagesScrollBox;
};
