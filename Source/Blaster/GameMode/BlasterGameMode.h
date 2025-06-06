// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

//namespace MatchState
//{
//	extern BLASTER_API const FName Cooldown; // Match duration has been reached. Display winner and begin timer
//}

class UShopPurchasable;
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;

	virtual void PlayerKilled(class ABlasterCharacter* KilledPlayer, class ABlasterPlayerController* KilledPlayerController, ABlasterPlayerController* AttackerController);

	UFUNCTION(BlueprintCallable)
	virtual void PlayerKilled(class ABlasterCharacter* KilledPlayer, class ABlasterPlayerController* KilledPlayerController);

	virtual void RespawnPlayer(ACharacter* KilledPlayer, AController* KilledPlayerController);

	/* Dynamic platforms that exist in the level, used for setting the ID for each of them */
	int32 DynamicPlatformsAmount = 0;


	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditAnywhere)
	uint32 MoneyPerKill = 10;

	void BroadcastChatMessage(FName Message);

	//UPROPERTY(EditDefaultsOnly)
	//float WarmupTime = 10.0f;
	//UPROPERTY(EditDefaultsOnly)
	//float MatchTime = 120.0f;
	//UPROPERTY(EditDefaultsOnly)
	//float CooldownTime = 10.0f;

	//float LevelStartingTime = 0.0f;

protected:
	virtual void BeginPlay() override;
	//virtual void OnMatchStateSet() override;

private: 
	//float CountdownTime = 0.0f;
};
