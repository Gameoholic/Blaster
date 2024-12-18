// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyHostWeapon.h"
#include "Components/WidgetInteractionComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"



ALobbyHostWeapon::ALobbyHostWeapon()
{
	WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteraction"));
	WidgetInteraction->SetupAttachment(RootComponent);

	WidgetInteraction->bEnableHitTesting = false; // This is enabled in BeginPlay()
	WidgetInteraction->InteractionDistance = 2500;
}

void ALobbyHostWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	WidgetInteraction->PressPointerKey(EKeys::LeftMouseButton);
	WidgetInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}

void ALobbyHostWeapon::BeginPlay()
{
	Super::BeginPlay();

	// We have to enable the component only for one player at a time. Otherwise issues are caused when it exists on multiple clients (because of the component). Since it's only used by the host, we'll enable it for them only.
	if (HasAuthority())
	{
		WidgetInteraction->bEnableHitTesting = true;
	}
}
