// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileWeapon.h"
#include "LobbyHostWeapon.generated.h"

class UWidgetInteractionComponent;
/**
 * 
 */
UCLASS()
class BLASTER_API ALobbyHostWeapon : public AProjectileWeapon
{
	GENERATED_BODY()
	
public:
	ALobbyHostWeapon();

	virtual void Fire(const FVector& HitTarget, bool bSilentFire) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* WidgetInteraction;
};
