// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterProjectileMoveComponent.h"

UBlasterProjectileMoveComponent::EHandleBlockingHitResult UBlasterProjectileMoveComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::Deflect; //AdvanceNextSubstep
}

void UBlasterProjectileMoveComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	Super::HandleImpact(Hit, TimeSlice, MoveDelta); // TODO, remove this if it causes issues but check all weapons including bounciness weapons
	// Overriding this makes the bullet continue even if it's interrupted. By default this method stops movement completely.
}




