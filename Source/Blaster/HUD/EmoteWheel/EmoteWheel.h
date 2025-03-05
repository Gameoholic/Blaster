// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EmoteWheel.generated.h"


USTRUCT(BlueprintType)
struct FEmoteType
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimSequence* Animation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class USoundCue* Sound;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UTexture2D* Icon;
};

/**
 *
 */
UCLASS()
class BLASTER_API UEmoteWheel : public UUserWidget
{
	GENERATED_BODY()


public:
	void ReleaseEmoteWheel();

	UFUNCTION(BlueprintImplementableEvent)
	void OnEmoteWheelReleased();

	UFUNCTION(BlueprintImplementableEvent)
	void OnEmoteWheelNextPagePressed();

	UFUNCTION(BlueprintImplementableEvent)
	void OnEmoteWheelPreviousPagePressed();

protected:
	virtual void NativeOnInitialized() override;
private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 SelectedEmoteIndex = -1;

	class ABlasterCharacter* BlasterCharacter = nullptr;
	class ABlasterPlayerController* BlasterPlayerController = nullptr;
};
