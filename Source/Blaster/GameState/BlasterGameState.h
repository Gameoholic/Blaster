// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABlasterPlayerState;
class AWeapon;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ServerUpdateTopScore(ABlasterPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<class ABlasterPlayerState*> TopScoringPlayers;

	FORCEINLINE TArray<TSubclassOf<AWeapon>> GetPurchasableWeapons() { return PurchasableWeapons; }
	FORCEINLINE TArray<TSubclassOf<AWeapon>> GetPurchasableWeapons2Temp() { return PurchasableWeapons2Temp; }
private:
	float TopScore = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Shop")
	TArray<TSubclassOf<AWeapon>> PurchasableWeapons;
	UPROPERTY(EditAnywhere, Category = "Shop")
	TArray<TSubclassOf<AWeapon>> PurchasableWeapons2Temp;
};
