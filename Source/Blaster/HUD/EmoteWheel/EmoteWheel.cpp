// Fill out your copyright notice in the Description page of Project Settings.


#include "EmoteWheel.h"
#include "Blaster/Character/BlasterCharacter.h"


void UEmoteWheel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	BlasterCharacter = Cast<ABlasterCharacter>(GetOwningPlayer());
}

void UEmoteWheel::ReleaseEmoteWheel()
{
	if (BlasterCharacter)
	{
		BlasterCharacter->SetIsEmoting(SelectedEmoteIndex != -1);
	}
}




