// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;


protected:
	virtual void BeginPlay() override;
	
	void DestroyTimerFinished();

	void DoRadialDamage();
	UPROPERTY(EditAnywhere, Category = "Radial Damage")
	float RadialDamageInnerRadius = 200.0f;
	UPROPERTY(EditAnywhere, Category = "Radial Damage")
	float RadialDamageOuterRadius = 500.0f;
	UPROPERTY(EditAnywhere, Category = "Radial Damage")
	// Exponential function falloff
	float RadialDamageFalloff = 1.0f;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	class UNiagaraComponent* TrailSystemComponent = nullptr;

	void StartDestroyTimer();
	void SpawnTrailSystem();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
	class UBlasterProjectileMoveComponent* ProjectileMovementComponent;

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent = nullptr;

	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
	float DestroyTime = 8.0f;


public:	

};
