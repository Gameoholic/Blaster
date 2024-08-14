// Fill out your copyright notice in the Description page of Project Settings.

// Name props to Elad Haleva

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DynamicPlatform.generated.h"

UENUM(BlueprintType)
enum class EDynamicPlatformChangeEndBehavior : uint8
{
	None UMETA(DisplayName = "None"),
	Continue UMETA(DisplayName = "Continue"),
	Loop UMETA(DisplayName = "Loop"),
	// also do pingpopFaster with a variable for directionback speed
	PingPong UMETA(DisplayName = "Ping Pong"),

	MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EDynamicPlatformPlayerOverlapBehavior : uint8
{
	None UMETA(DisplayName = "None"),
	Enable UMETA(DisplayName = "Enable"),
	Disable UMETA(DisplayName = "Disable"),
	/** Will only move the platform one way while overlapping, and will return opposite when not overlapping. Only works when end behavior is set to ping pong.*/
	PingPongEnableOneWay UMETA(DisplayName = "Ping Pong Enable OneWay"),



	MAX UMETA(DisplayName = "DefaultMAX")
};


/**
* Contains data regarding the platform's state that should be PRC'd down to clients.
*/
USTRUCT()
struct FDynamicPlatformReplicationData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Id; // YT: Maps can't be rpc'd, therefore can't use id as key so we store it here
	
	UPROPERTY()
	float CurrentChangeTime;
	UPROPERTY()
	bool bOppositeDirection;
	UPROPERTY()
	bool bActive;
	UPROPERTY()
	FDateTime TimeInServer;

	FDynamicPlatformReplicationData()
	{

	}
};

UCLASS()
class BLASTER_API ADynamicPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	ADynamicPlatform();
	virtual void Tick(float DeltaTime) override;
	/** 
	* Updates the platform's state with the replicated data from the server.
	* @param TimeToRespondToRequest How many seconds it took from the moment we requested the state to the moment the server provided it.
	*/
	void ClientUpdateDynamicPlatformState(
		FDynamicPlatformReplicationData ReplicationData, 
		float TimeToRespondToRequest
	);

	void ServerReplicatePlatformData();
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* BoxCollision;

	FDateTime CurrentTimeReceive = NULL;

	/// --------------- GENERAL UPROPS -------------------

	/** The duration of the platform changing its properties (position, rotation, scale). */
	UPROPERTY(EditAnywhere, Category = "Dynamic Platform", meta = (ClampMin = 0.1f))
	float ChangeTime = 10.0f;

	/** The change time to start in, portion. */
	UPROPERTY(EditAnywhere, Category = "Dynamic Platform", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float ChangeTimeHeadstart = 0.0f;

	/** The behavior of the platform when it ends changing. */
	UPROPERTY(EditAnywhere, Category = "Dynamic Platform")
	EDynamicPlatformChangeEndBehavior ChangeEndBehavior = EDynamicPlatformChangeEndBehavior::Continue;

	/** The behavior of the platform when player overlaps. */
	UPROPERTY(EditAnywhere, Category = "Dynamic Platform")
	EDynamicPlatformPlayerOverlapBehavior PlayerOverlapBehavior = EDynamicPlatformPlayerOverlapBehavior::None;

	/// --------------- MOVEMENT UPROPS -------------------
/** Where the platform will move to, relative to its start location. */
	UPROPERTY(EditAnywhere, Category = "Moving Platform")
	FVector PlatformRelativeMovement = FVector(0, 0, 0);

	/** The ease (Exp), interpolating the movement with an ease-in and ease-out. Set to 0 to none. */
	UPROPERTY(EditAnywhere, Category = "Moving Platform")
	float PlatformMoveEase = 1.0f;

	/** The portion of the movement to start in. */
	UPROPERTY(EditAnywhere, Category = "Moving Platform", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float PlatformMovementHeadstart = 0.0f;


	/// --------------- ROTATION UPROPS -------------------


	/** Where the platform will rotate to, relative to its start rotation. */
	UPROPERTY(EditAnywhere, Category = "Rotating Platform")
	FRotator PlatformRelativeRotation = FRotator(0, 0, 0);

	/** The ease (Exp), controlling the degree of the movement curve. Set to 0 to none. */
	UPROPERTY(EditAnywhere, Category = "Rotating Platform")
	float PlatformRotateEase = 1.0f;

	/** The portion of the movement to start in. */
	UPROPERTY(EditAnywhere, Category = "Rotating Platform", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float PlatformRotationHeadstart = 0.0f;

	/// --------------- Scale UPROPS -------------------


/** Where the platform will scale to, relative to its start scale. */
	UPROPERTY(EditAnywhere, Category = "Scaling Platform")
	FVector PlatformRelativeScale = FVector(0, 0, 0);

	/** The ease (Exp), controlling the degree of the movement curve. Set to 0 to none. */
	UPROPERTY(EditAnywhere, Category = "Scaling Platform")
	float PlatformScaleEase = 1.0f;

	/** The portion of the movement to start in. */
	UPROPERTY(EditAnywhere, Category = "Scaling Platform", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float PlatformScaleHeadstart = 0.0f;



	/** Unique Id of dynamic platform, starts with 0. */
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Id)
	int32 Id = -155;
	UFUNCTION()
	void OnRep_Id();

	/** Whether the platform should currently change or not. */
	bool bActive = true;

	/** The seconds that have passed since starting to change the platform's properties, until it finishes. */
	float CurrentChangeTime = 0.0f;

	/** Whether we're going in the opposite direction of the original movement. */
	bool bOppositeDirection = false;

	/** The cycle index, only incremented if end mode is set to Continue */
	int32 CycleIndex = 0;

	FVector GetLocationForCurrentChangeTime();
	FRotator GetRotationForCurrentChangeTime();
	FVector GetScaleForCurrentChangeTime();

	void MovePlatform(float DeltaTime);
	void RotatePlatform(float DeltaTime);
	void ScalePlatform(float DeltaTime);

	FVector StartLocation;
	FRotator StartRotation;
	FVector StartScale;


	UFUNCTION()
	void ServerOnPlatformBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
	void ServerOnPlatformEndOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex
	);
	int32 ServerPlayersOnPlatform = 0;

	void ServerOnDynamicPlatformPlayerBeginOverlap();
	void ServerOnDynamicPlatformPlayerEndOverlap();


	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceivePlatformReplicationData(FDynamicPlatformReplicationData ReplicationData);

public:
	FORCEINLINE const int32 GetId() { return Id; };
	FORCEINLINE const float GetCurrentChangeTime() { return CurrentChangeTime; };
	FORCEINLINE const bool GetIsOppositeDirection() { return bOppositeDirection; };
	FORCEINLINE const bool GetIsActive() { return bActive; };
};
