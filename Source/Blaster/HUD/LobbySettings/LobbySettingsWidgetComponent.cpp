// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbySettingsWidgetComponent.h"
#include "Blaster/GameMode/LobbyGameMode.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"




void ULobbySettingsWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);
}

void ULobbySettingsWidgetComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULobbySettingsWidgetComponent, SelectedMapPath);
}


void ULobbySettingsWidgetComponent::SelectMap(FString SelectedMapPathString)
{
	// Path for some reason contains . and then repeats itself so we get rid of the dot and everything after it
	int32 DotIndex;
	SelectedMapPathString.FindLastChar('.', DotIndex);
	SelectedMapPath = SelectedMapPathString.Left(DotIndex);
}

void ULobbySettingsWidgetComponent::StartGame()
{
	if (SelectedMapPath == TEXT(""))
	{
		return;
	}

	if (GetLobbyGameMode())
	{
		LobbyGameMode->TravelToMap(SelectedMapPath);
	}
}

void ULobbySettingsWidgetComponent::KickPlayer(APlayerState* PlayerToBeKicked)
{
	if (GetLobbyGameMode())
	{
		APlayerController* Controller = PlayerToBeKicked->GetPlayerController();
		if (Controller)
		{
			LobbyGameMode->KickPlayer(Controller);
		}
	}
}

void ULobbySettingsWidgetComponent::OnRep_SelectedMapPath(FString LastMapPath)
{
	OnReplicatedSelectedMapPath();
}



ALobbyGameMode* ULobbySettingsWidgetComponent::GetLobbyGameMode()
{
	if (LobbyGameMode)
	{
		return LobbyGameMode;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode();
		if (GameMode != nullptr)
		{
			LobbyGameMode = Cast<ALobbyGameMode>(GameMode);
			return LobbyGameMode;
		}
	}
	return nullptr;
}



