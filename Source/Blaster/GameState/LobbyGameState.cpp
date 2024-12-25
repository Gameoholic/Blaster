// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"



void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, HostPlayerName);
}

void ALobbyGameState::BeginPlay()
{
	if (HasAuthority())
	{
		HostPlayerName = UGameplayStatics::GetPlayerController(this, 0)->PlayerState.Get()->GetPlayerName();
	}
}
