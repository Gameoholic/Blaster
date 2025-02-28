// Fill out your copyright notice in the Description page of Project Settings.


#include "EmoteWheel.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"


void UEmoteWheel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BlasterPlayerController = Cast<ABlasterPlayerController>(GetOwningPlayer());
	if (BlasterPlayerController)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(BlasterPlayerController->GetCharacter());
	}
}

void UEmoteWheel::ReleaseEmoteWheel()
{
	if (BlasterCharacter)
	{
		BlasterCharacter->SetIsEmoting(SelectedEmoteIndex != -1);
		BlasterCharacter->SetSelectedEmoteAnimation(SelectedEmoteAnimation);
		if (SelectedEmoteIndex != -1)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0, FColor::Red, FString::Printf(TEXT("Emote: true")));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0, FColor::Red, FString::Printf(TEXT("Emote: false")));
		}
	}

	OnEmoteWheelRelease();
}




