// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "BlasterProjectileMoveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	// Destroy immediately if hit player
	if (ImpactResult.GetActor()->GetClass()->IsChildOf<ABlasterCharacter>() && ImpactResult.GetActor() != GetOwner())
	{
		Destroy();
		return;
	}

	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
	}
}

void AProjectileGrenade::Destroyed()
{
	DoRadialDamage();
	Super::Destroyed();
}


