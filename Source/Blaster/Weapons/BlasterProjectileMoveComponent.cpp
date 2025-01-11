// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterProjectileMoveComponent.h"

UBlasterProjectileMoveComponent::EHandleBlockingHitResult UBlasterProjectileMoveComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Blue, TEXT("HandleBlockingHit"));
	return EHandleBlockingHitResult::AdvanceNextSubstep; //AdvanceNextSubstep
}

void UBlasterProjectileMoveComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Blue, TEXT("HandleImpact"));
	// Projectile should not stop, only explode when CollisionBox detects a hit
}
