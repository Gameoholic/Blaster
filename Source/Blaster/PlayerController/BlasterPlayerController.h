// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"


class UWidget;
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float PreviousHealth, float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDKills(int32 Kills);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 WeaponRemainingAmmo, int32 WeaponMagCapacity);
	void SetHUDMainWeapon(FString WeaponName, FSlateBrush WeaponIcon);
	void SetHUDSecondaryWeapon(FString WeaponName, FSlateBrush WeaponIcon);
	void SetHUDItem1(FString ItemName, FSlateBrush ItemIcon);
	void SetHUDItem2(FString ItemName, FSlateBrush ItemIcon);
	void ToggleChat();
	void SendChatMessage(FName Message);
	//void SetHUDMatchCountdown(float CountdownTime);
	//void SetHUDAnnouncementCountdown(float CountdownTime);

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable)
	void ClientSendMessage(FName Message);

	//virtual float GetServerTime(); // Synced with server world clock

	//virtual void ReceivedPlayer() override; // Earliest time we can get world time

	//UPROPERTY(EditAnywhere, Category = Time)
	//float TimeSyncFrequency = 5.0f; // How often to sync client-server worldtime

	//float TimeSyncElapsedSeconds = 0.0f;
	//void CheckTimeSync(float DeltaTime);

	//void OnMatchStateSet(FName State);
	//void HandleMatchHasStarted();

	//void HandleCooldown();

protected:
	virtual void SetupInputComponent() override;

	virtual void BeginPlay() override;
	//void SetHUDTime();

	///* Sync time between client and server */
	//// Requests the current server time, passing in the client's time when the request was sent
	//UFUNCTION(Server, Reliable)
	//void ServerRequestServerTime(float TimeOfClientRequest);

	//// Reports the current server time to the client in response to ServerRequestServerTime
	//UFUNCTION(Client, Reliable)
	//void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//float ClientServerDelta = 0.0f; // Difference between client and server time

	void PollInit();

	//UFUNCTION(Server, Reliable)
	//void ServerCheckMatchState();

	//UFUNCTION(Client, Reliable)
	//void ClientJoinMidgame(FName StateOfMatch, float ProvidedWarmupTime, float ProvidedMatchTime, float ProvidedStartingTime);

private:
	bool IsHUDValid();
	class ABlasterHUD* BlasterHUD = nullptr;

	bool bChatMessagesLoadedFromGameInstance = false;

	//float LevelStartingTime = 0.0f;
	//float MatchTime = 0.0f;
	//float WarmupTime = 0.0f;
	//uint32 CountdownInt = 0;

	//UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	//FName MatchState;

	//UFUNCTION()
	//void OnRep_MatchState();

	void TogglePauseMenu();

	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> PauseMenuWidget;


};
