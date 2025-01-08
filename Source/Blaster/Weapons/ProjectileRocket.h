// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

	virtual void Destroyed() override;
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit) override;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	void DestroyTimerFinished();

	class UNiagaraComponent* TrailSystemComponent = nullptr;

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	UAudioComponent* ProjectileLoopComponent = nullptr;
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;
	
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditAnywhere, Category = Rocket)
	float DamageInnerRadius = 250.0f;

	UPROPERTY(EditAnywhere, Category = Rocket)
	float DamageOuterRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = Rocket)
	float DamageFalloff = 1.0f; // exponential function falloff

	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
	float DestroyTime = 8.0f;
};
