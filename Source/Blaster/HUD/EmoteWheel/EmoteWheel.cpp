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
		if (BlasterCharacter->GetSelectedEmoteIndex() != SelectedEmoteIndex) // If player selected the same emote, just close the menu as if nothing changed
		{
			BlasterCharacter->SetSelectedEmoteIndex(SelectedEmoteIndex);
		}
	}

	OnEmoteWheelReleased();
}






