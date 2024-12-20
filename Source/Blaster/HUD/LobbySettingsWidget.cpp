// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbySettingsWidget.h"
#include "Blaster/GameMode/LobbyGameMode.h"
#include "Net/UnrealNetwork.h"


void ULobbySettingsWidget::SelectMap(FString SelectedMapPathString)
{
	// Path for some reason contains . and then repeats itself so we get rid of the dot and everything after it
	int32 DotIndex;
	SelectedMapPathString.FindLastChar('.', DotIndex);
	SelectedMapPath = SelectedMapPathString.Left(DotIndex);
}

void ULobbySettingsWidget::StartGame()
{
	if (SelectedMapPath == TEXT(""))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{

		AGameModeBase* GameMode = World->GetAuthGameMode();
		if (GameMode != nullptr)
		{

			ALobbyGameMode* LobbyGameMode = Cast<ALobbyGameMode>(GameMode);
			if (LobbyGameMode != nullptr)
			{
				LobbyGameMode->TravelToMap(SelectedMapPath);
			}
		}
	}
}



