// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "GameFramework/PlayerController.h"
#include "Blaster/MultiplayerSessions/MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"

void UPauseMenu::SetupMenu()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	// Display menu to player
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	// Set up multiplayer function delegates
	if (GetMultiplayerSessionsSubsystem())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UPauseMenu::OnDestroySession);
	}
}

UMultiplayerSessionsSubsystem* UPauseMenu::GetMultiplayerSessionsSubsystem()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = MultiplayerSessionsSubsystem == nullptr ? GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>() : MultiplayerSessionsSubsystem;
	}
	return MultiplayerSessionsSubsystem;
}



void UPauseMenu::TearDownMenu()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}

	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UPauseMenu::OnDestroySession);
	}

	PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
}

void UPauseMenu::ReturnToMainMenu()
{
	if (GetMultiplayerSessionsSubsystem())
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

void UPauseMenu::ExitGame()
{
	if (GetMultiplayerSessionsSubsystem())
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
	// leave game on OnDestroySession()
}


void UPauseMenu::OnDestroySession(bool bWasSuccessful)
{

	if (!bWasSuccessful)
	{
		//re-enable button / LOADING menu
		return;
	}
	
	// If is server, return to main menu and shut down all clients
	if (PlayerController->HasAuthority())
	{
		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost(); // returns to main menu, shut down clients and tell them to go to main menu
		}
	}
	else
	{
		PlayerController->ClientReturnToMainMenuWithTextReason(FText());
	}




}


