// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

class UMultiplayerSessionsSubsystem;
class APlayerController;

/**
 * 
 */
UCLASS()
class BLASTER_API UPauseMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetupMenu();
	UFUNCTION(BlueprintCallable)
	void TearDownMenu();

	UFUNCTION(BlueprintCallable)
	void ReturnToMainMenu();
	UFUNCTION(BlueprintCallable)
	void ExitGame();

private:
	// Not safe to call directly. Use GetMultiplayerSessionsSubsystem() then check if null
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = nullptr;
	UMultiplayerSessionsSubsystem* GetMultiplayerSessionsSubsystem();
	APlayerController* PlayerController = nullptr;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);


};
