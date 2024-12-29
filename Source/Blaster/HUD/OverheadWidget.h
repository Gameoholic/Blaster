// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget)) // Bind to text block in widget blueprint
	class UTextBlock* DisplayText; // Needs to have same name as variable in widget blueprint

	UPROPERTY(meta = (BindWidget))
	class UBorder* TextBorder;

	void SetDisplayText(FString TextToDisplay);
	// Appends host icon to the widget
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetIsHost();

protected:
	virtual void NativeDestruct() override;
};
