// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MovingPlatform.generated.h"

UCLASS()
class BLASTER_API AMovingPlatform : public AActor
{
	GENERATED_BODY()
	
public:
	AMovingPlatform();

	virtual void Tick(float DeltaTime) override;
private:
	virtual void BeginPlay() override;

	void MovePlatform(float DeltaTime);
	void RotatePlatform(float DeltaTime);

	/// --------------- MOVEMENT -------------------
	/** Where the platform will move to, relative to its start location. */
	UPROPERTY(EditAnywhere, Category = "Moving Platform")
	FVector PlatformRelativeMovement = FVector(0, 0, 1000);

	/** The ease (Exp), controlling the degree of the movement curve. Set to 0 to none. */
	UPROPERTY(EditAnywhere, Category = "Moving Platform")
	float PlatformMoveCurveEase = 1.0f;

	/** The duration of the platform moving one way. */
	UPROPERTY(EditAnywhere, Category = "Moving Platform")
	float PlatformMoveDuration = 10.0f;

	/** The portion of the movement to start in. */
	UPROPERTY(EditAnywhere, Category = "Moving Platform", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float PlatformMovementHeadstart = 0.0f;

	FVector StartLocation;
	int32 MovementDirection = 1; // -1 if need to reverse direction
	float MovementAlpha = 0.0f; // alpha is progress between 0 and 1

	/// --------------- ROTATION -------------------


	/** Where the platform will rotate to, relative to its start rotation. */
	UPROPERTY(EditAnywhere, Category = "Rotating Platform")
	FRotator PlatformRelativeRotation = FRotator(0, 0, 0);

	/** The ease (Exp), controlling the degree of the movement curve. Set to 0 to none. */
	UPROPERTY(EditAnywhere, Category = "Rotating Platform")
	float PlatformRotateCurveEase = 1.0f;

	/** The duration of the platform moving one way. */
	UPROPERTY(EditAnywhere, Category = "Rotating Platform")
	float PlatformRotateDuration = 10.0f;

	/** The portion of the movement to start in. */
	UPROPERTY(EditAnywhere, Category = "Rotating Platform", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float PlatformRotationHeadstart = 0.0f;

	FRotator StartRotation;
	int32 RotationDirection = 1; // -1 if need to reverse direction
	float RotationAlpha = 0.0f; // alpha is progress between 0 and 1



};
