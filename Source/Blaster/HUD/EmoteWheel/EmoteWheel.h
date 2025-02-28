// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EmoteWheel.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UEmoteWheel : public UUserWidget
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnEmoteWheelRelease();
private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 SelectedEmoteIndex = -1;



};
