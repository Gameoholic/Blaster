// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	void ServerAddToScore(float ScoreAmount);
	void DisplayUpdatedScore();

	void ServerAddToKills(int KillsAmount);
	void DisplayUpdatedKills();

	void ServerAddToDeaths(int DeathsAmount);
	void DisplayUpdatedDeaths();


	/** 
	* Replication notifies
	*/
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Deaths();
	UFUNCTION()
	virtual void OnRep_Kills();

private:
	class ABlasterCharacter* Character = nullptr;
	class ABlasterPlayerController* Controller = nullptr;
	bool IsControllerValid();

	UPROPERTY(ReplicatedUsing = OnRep_Kills)
	int32 Kills;
	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;
};
