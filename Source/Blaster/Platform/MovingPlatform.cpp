// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatform.h"
#include "Components/InterpToMovementComponent.h"

AMovingPlatform::AMovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	StartRotation = GetActorRotation();
	MovementAlpha = PlatformMovementHeadstart;
	RotationAlpha = PlatformRotationHeadstart;
}

void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovePlatform(DeltaTime);
	RotatePlatform(DeltaTime);
}

void AMovingPlatform::MovePlatform(float DeltaTime)
{
	MovementAlpha += DeltaTime / PlatformMoveDuration * MovementDirection; //alpha is progress between 0 and 1.

	FVector DeltaMovement = FVector(
		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeMovement.X), MovementAlpha, PlatformMoveCurveEase),
		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeMovement.Y), MovementAlpha, PlatformMoveCurveEase),
		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeMovement.Z), MovementAlpha, PlatformMoveCurveEase)
	);


	if (MovementAlpha >= 1.0f)
	{
		MovementDirection = -1; // reverse movement direction
		SetActorLocation(StartLocation + PlatformRelativeMovement); // reset exact loc
	}
	else if (MovementAlpha <= 0.0f)
	{
		MovementDirection = 1; // reverse movement direction back to original
		SetActorLocation(StartLocation); // reset exact loc
	}
	else
	{
		SetActorLocation(StartLocation + DeltaMovement);
	}
}
void AMovingPlatform::RotatePlatform(float DeltaTime)
{
	RotationAlpha += DeltaTime / PlatformRotateDuration * RotationDirection; //alpha is progress between 0 and 1.

	FRotator DeltaRotation = FRotator(
		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeRotation.Pitch), RotationAlpha, PlatformRotateCurveEase),
		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeRotation.Yaw), RotationAlpha, PlatformRotateCurveEase),
		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeRotation.Roll), RotationAlpha, PlatformRotateCurveEase)
	);


	if (RotationAlpha >= 1.0f)
	{
		RotationDirection = -1; // reverse rotation direction
		SetActorRotation(StartRotation + PlatformRelativeRotation); // reset exact rotation
	}
	else if (RotationAlpha <= 0.0f)
	{
		RotationDirection = 1; // reverse rotation direction back to original
		SetActorRotation(StartRotation); // reset exact loc
	}
	else
	{
		SetActorRotation(StartRotation + DeltaRotation);
	}
}




//void AMovingPlatform::RotatePlatform(float DeltaTime)
//{
//	AddActorLocalRotation(PlatformRotationVelocity * DeltaTime);
//}



