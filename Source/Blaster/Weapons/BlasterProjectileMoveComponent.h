// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BlasterProjectileMoveComponent.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterProjectileMoveComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
	
protected:
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;



};
