// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority())
	{
		return;
	}

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation(); // From muzzle flash socket to hit location from TraceUnderCrosshairs()
		FRotator TargetRot = ToTarget.Rotation();

		if (ProjectileClass && InstigatorPawn)
		{
			UWorld* World = GetWorld();
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;
			
			World->SpawnActor<AProjectile>(
				ProjectileClass,
				SocketTransform.GetLocation(),
				TargetRot,
				SpawnParameters
			);
			GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Red, FString::Printf(TEXT("%f, %f, %f"), SocketTransform.GetLocation().X, SocketTransform.GetLocation().Y, SocketTransform.GetLocation().Z));
			GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Red, FString::Printf(TEXT("%f, %f, %f"), HitTarget.X, HitTarget.Y, HitTarget.Z));

			
			
			DrawDebugLine(GetWorld(), SocketTransform.GetLocation(), HitTarget, FColor::Red, false, 30.0f);
			DrawDebugSphere(GetWorld(), SocketTransform.GetLocation(), 10.0f, 6, FColor::Blue, false, 30.0f);
		}
	}
}
