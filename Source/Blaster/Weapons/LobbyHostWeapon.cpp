// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyHostWeapon.h"
#include "Components/WidgetInteractionComponent.h"



ALobbyHostWeapon::ALobbyHostWeapon()
{
	WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction"));
	WidgetInteraction->SetupAttachment(RootComponent);

	WidgetInteraction->bEnableHitTesting = true;
	WidgetInteraction->InteractionDistance = 2500;
}

void ALobbyHostWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	WidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
	WidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}
