// Fill out your copyright notice in the Description page of Project Settings.



#include "DynamicPlatform.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameInstance/BlasterGameInstance.h"
#include "Components/BoxComponent.h"


ADynamicPlatform::ADynamicPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BoxCollision->SetupAttachment(Mesh);
	BoxCollision->SetGenerateOverlapEvents(true);
	BoxCollision->SetCollisionProfileName(FName("OverlapOnlyPawn"));

	bReplicates = true; // Necessary for Id replication, do not replicate movement

	
}

void ADynamicPlatform::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	//DOREPLIFETIME_CONDITION(ADynamicPlatform, Id, COND_InitialOnly); // COND_Initial - Will only replicate once.
}

void ADynamicPlatform::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	StartRotation = GetActorRotation();
	StartScale = GetActorScale();

	if (HasAuthority())
	{
		// Set (and replicate) Id
		ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(GetWorld()->GetAuthGameMode());
		if (BlasterGameMode)
		{
			Id = BlasterGameMode->DynamicPlatformsAmount;
			BlasterGameMode->DynamicPlatformsAmount += 1;
			//UE_LOG(LogTemp, Warning, TEXT("ID IS ON DYNAMICPLATFORM: %d"), Id);
		}

		BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ADynamicPlatform::ServerOnPlatformBeginOverlap);
		BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ADynamicPlatform::ServerOnPlatformEndOverlap);
	}
	else
	{
		// Add to number of unloaded platforms (platforms that still do not have an ID set)
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[client] beginplay() Platform id is %d"), Id));
		UBlasterGameInstance* GameInstance = Cast<UBlasterGameInstance>(GetWorld()->GetGameInstance());
		if (GameInstance)
		{
			GameInstance->NoIdDynamicPlatforms += 1;
		}
	}

	// We only want the platform to activate once player steps on it
	if (PlayerOverlapBehavior == EDynamicPlatformPlayerOverlapBehavior::Enable || PlayerOverlapBehavior == EDynamicPlatformPlayerOverlapBehavior::PingPongEnableOneWay)
	{
		bActive = false;
	}
	CurrentChangeTime = ChangeTime * ChangeTimeHeadstart;
}

void ADynamicPlatform::ClientUpdateDynamicPlatformState(FDynamicPlatformReplicationData ReplicationData, float TimeToRespondToRequest)
{
	//GEngine->AddOnScreenDebugMessage(-1, 7.0f, FColor::White, FString::Printf(TEXT("receive new timings: %f, with delay of %f"), ReplicationData.CurrentChangeTime, TimeToRespondToRequest / 2.0f));
	//CurrentChangeTime = ReplicationData.CurrentChangeTime + TimeToRespondToRequest / 2.0f; // Get estimated one-way trip from server to client duration, because CurrentChangeTime is outdated by the time client receives it
	//bOppositeDirection = ReplicationData.bOppositeDirection;
	//bActive = ReplicationData.bActive;
}


void ADynamicPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bActive)
	{
		if (CurrentTimeReceive != NULL)
		{
			FDateTime CurrentTime = FDateTime::UtcNow();

			FTimespan TimeDifference = FTimespan(CurrentTime.GetTicks() - CurrentTimeReceive.GetTicks());
			CurrentChangeTime += TimeDifference.GetTotalMicroseconds() / 1000000.0f;
			CurrentTimeReceive = NULL;
		}
		else
		{
			CurrentChangeTime += DeltaTime;
		}


		while (CurrentChangeTime >= ChangeTime) // Lag from response from server to client could result in CurrentChangeTime being multiple times bigger than ChangeTime, so we use loop instead of if
		{
			if (PlayerOverlapBehavior != EDynamicPlatformPlayerOverlapBehavior::PingPongEnableOneWay)
			{
				// Normal behavior
				switch (ChangeEndBehavior)
				{
				case EDynamicPlatformChangeEndBehavior::PingPong:
					bOppositeDirection = !bOppositeDirection;
					break;
				case EDynamicPlatformChangeEndBehavior::Continue:
					CycleIndex++;
					break;
				case EDynamicPlatformChangeEndBehavior::None:
					bActive = false;
					break;
				}
			}
			else
			{
				// Ping pong only one way does not need us to change direction or activate etc. here
				bActive = false;
				bOppositeDirection = !bOppositeDirection;
			}

			CurrentChangeTime -= ChangeTime;
		}

		MovePlatform(DeltaTime);
		RotatePlatform(DeltaTime);
		ScalePlatform(DeltaTime);
	}
}

void ADynamicPlatform::OnRep_Id()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[client] onrep() Platform id is %d"), Id));

	// Decrease amount of un-id'd platforms
	UBlasterGameInstance* GameInstance = Cast<UBlasterGameInstance>(GetWorld()->GetGameInstance());
	//yt: make sure game instance is set through project settings.

	if (GameInstance)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[client] found gameinstance, %d"), GameInstance->NoIdDynamicPlatforms));
		GameInstance->NoIdDynamicPlatforms -= 1;
		// If all platforms are now id's
		if (GameInstance->NoIdDynamicPlatforms == 0)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[client] non id's is now 0"), Id));
			// Get the blaster character that's locally controlled
			TArray<AActor*> CharacterActors;
			UGameplayStatics::GetAllActorsOfClass(this, ABlasterCharacter::StaticClass(), CharacterActors);
			for (AActor* CharacterActor : CharacterActors)
			{
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(CharacterActor);
				if (BlasterCharacter && BlasterCharacter->IsLocallyControlled())
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[client] about to request platform states from server")));
					// Now that we got all the platforms id'd, we request the platform states from server
					BlasterCharacter->ClientRequestDynamicPlatformStates();
				}
			}
		}
	}
}


void ADynamicPlatform::ServerOnPlatformBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ServerPlayersOnPlatform += 1;
	if (ServerPlayersOnPlatform == 1)
	{
		ServerOnDynamicPlatformPlayerBeginOverlap();
	}
}

void ADynamicPlatform::ServerOnPlatformEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ServerPlayersOnPlatform -= 1;
	if (ServerPlayersOnPlatform == 0)
	{
		ServerOnDynamicPlatformPlayerEndOverlap();
	}
}


void ADynamicPlatform::ServerOnDynamicPlatformPlayerBeginOverlap()
{
	switch (PlayerOverlapBehavior)
	{
	case EDynamicPlatformPlayerOverlapBehavior::Disable:
		bActive = false;
		break;
	case EDynamicPlatformPlayerOverlapBehavior::Enable:
		bActive = true;
		break;
	case EDynamicPlatformPlayerOverlapBehavior::PingPongEnableOneWay:
		bActive = true;
		// If platform is already returning when we step on it, we want to flip its change time as well as the direction
		if (bOppositeDirection)
		{
			CurrentChangeTime = (ChangeTime - CurrentChangeTime);
		}
		bOppositeDirection = false;
		break;
	}
	if (PlayerOverlapBehavior != EDynamicPlatformPlayerOverlapBehavior::None) // Only needs to replicate if platform has player overlap behavior, otherwise nothing's changed
	{
		ServerReplicatePlatformData();
	}
}

void ADynamicPlatform::ServerReplicatePlatformData()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("Replicating on server... "));
	FDynamicPlatformReplicationData ReplicationData;
	ReplicationData.bActive = bActive;
	ReplicationData.bOppositeDirection = bOppositeDirection;
	ReplicationData.CurrentChangeTime = CurrentChangeTime;
	ReplicationData.TimeInServer = FDateTime::UtcNow();
	MulticastReceivePlatformReplicationData(ReplicationData);
}
void ADynamicPlatform::ServerOnDynamicPlatformPlayerEndOverlap()
{
	switch (PlayerOverlapBehavior)
	{
	case EDynamicPlatformPlayerOverlapBehavior::Disable:
		bActive = true;
		break;
	case EDynamicPlatformPlayerOverlapBehavior::Enable:
		bActive = false;
		break;
	case EDynamicPlatformPlayerOverlapBehavior::PingPongEnableOneWay:
		bActive = true;
		// If platform is not going opposite when we step on it, we want to flip its change time as well as the direction
		if (!bOppositeDirection)
		{
			CurrentChangeTime = (ChangeTime - CurrentChangeTime);
		}
		bOppositeDirection = true;
		break;
	}
	ServerReplicatePlatformData();
}





void ADynamicPlatform::MovePlatform(float DeltaTime)
{
	SetActorLocation(GetLocationForCurrentChangeTime());
}

FVector ADynamicPlatform::GetLocationForCurrentChangeTime()
{
	float ChangeTimeAfterApplyingDirection = bOppositeDirection ? ChangeTime - CurrentChangeTime : CurrentChangeTime;
	float ChangeProgress = ChangeTimeAfterApplyingDirection / ChangeTime;
	float InterpolatedChangeProgress = PlatformMoveEase == 0 ? ChangeProgress : 
		FMath::InterpEaseInOut(0.0f, 1.0f, ChangeProgress, PlatformMoveEase);
	FVector LeftoverDistanceFromPreviousCycles = PlatformRelativeMovement * CycleIndex;
	if (HasAuthority())
	{
		//GEngine->AddOnScreenDebugMessage(0, 0.5f, FColor::Blue, FString::Printf(TEXT("[auth] ChangeProgress: %f"), ChangeProgress));
		//GEngine->AddOnScreenDebugMessage(1, 0.5f, FColor::Blue, FString::Printf(TEXT("[auth] InterpolatedChangeProgress: %f"), InterpolatedChangeProgress));
	}


	if (!HasAuthority())
	{
		//GEngine->AddOnScreenDebugMessage(2, 0.5f, FColor::Blue, FString::Printf(TEXT("[nonauth] ChangeProgress: %f"), ChangeProgress));
		//GEngine->AddOnScreenDebugMessage(3, 0.5f, FColor::Blue, FString::Printf(TEXT("[nonauth] InterpolatedChangeProgress: %f"), InterpolatedChangeProgress));
	}

	return StartLocation + PlatformRelativeMovement * InterpolatedChangeProgress + LeftoverDistanceFromPreviousCycles;
}



void ADynamicPlatform::RotatePlatform(float DeltaTime)
{
	SetActorRotation(GetRotationForCurrentChangeTime());
}

FRotator ADynamicPlatform::GetRotationForCurrentChangeTime()
{
	float ChangeTimeAfterApplyingDirection = bOppositeDirection ? ChangeTime - CurrentChangeTime : CurrentChangeTime;
	float ChangeProgress = ChangeTimeAfterApplyingDirection / ChangeTime;
	float InterpolatedChangeProgress = PlatformRotateEase == 0 ? ChangeProgress :
		FMath::InterpEaseInOut(0.0f, 1.0f, ChangeProgress, PlatformRotateEase);
	FRotator LeftoverRotationFromPreviousCycles = PlatformRelativeRotation * CycleIndex;

	return StartRotation + PlatformRelativeRotation * InterpolatedChangeProgress + LeftoverRotationFromPreviousCycles;
}

void ADynamicPlatform::ScalePlatform(float DeltaTime)
{
	SetActorScale3D(GetScaleForCurrentChangeTime());
}

void ADynamicPlatform::MulticastReceivePlatformReplicationData_Implementation(FDynamicPlatformReplicationData ReplicationData)
{
	CurrentTimeReceive = FDateTime::UtcNow();
	FTimespan ServerClientTimeDifference = FTimespan(CurrentTimeReceive.GetTicks() - ReplicationData.TimeInServer.GetTicks());
	//GEngine->AddOnScreenDebugMessage(-1, 7.0f, FColor::White, FString::Printf(TEXT("MULTICAST receive new timings: %f, with delay of %f ms"), ReplicationData.CurrentChangeTime, ServerClientTimeDifference.GetTotalMilliseconds()));


	CurrentChangeTime = ReplicationData.CurrentChangeTime + ServerClientTimeDifference.GetTotalMicroseconds() / 1000000.0f; // TODO: THIS ISN'T ACCURATE AND DOESN;'T ACCOUNT FOR SERVER PING DELAY!!!
	bOppositeDirection = ReplicationData.bOppositeDirection;
	bActive = ReplicationData.bActive;
}

FVector ADynamicPlatform::GetScaleForCurrentChangeTime()
{
	float ChangeTimeAfterApplyingDirection = bOppositeDirection ? ChangeTime - CurrentChangeTime : CurrentChangeTime;
	float ChangeProgress = ChangeTimeAfterApplyingDirection / ChangeTime;
	float InterpolatedChangeProgress = PlatformMoveEase == 0 ? ChangeProgress :
		FMath::InterpEaseInOut(0.0f, 1.0f, ChangeProgress, PlatformScaleEase);
	FVector LeftoverScaleFromPreviousCycles = PlatformRelativeScale * CycleIndex;

	return StartScale + PlatformRelativeScale * InterpolatedChangeProgress + LeftoverScaleFromPreviousCycles;
}





//void ADynamicPlatform::MovePlatform(float DeltaTime)
//{
//	MovementAlpha += DeltaTime / PlatformMoveDuration * MovementDirection; //alpha is progress between 0 and 1.
//
//	FVector DeltaMovement = FVector(
//		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeMovement.X), MovementAlpha, PlatformMoveCurveEase),
//		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeMovement.Y), MovementAlpha, PlatformMoveCurveEase),
//		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeMovement.Z), MovementAlpha, PlatformMoveCurveEase)
//	);
//
//
//	if (MovementAlpha >= 1.0f)
//	{
//		MovementDirection = -1; // reverse movement direction
//		SetActorLocation(StartLocation + PlatformRelativeMovement); // reset exact loc
//	}
//	else if (MovementAlpha <= 0.0f)
//	{
//		MovementDirection = 1; // reverse movement direction back to original
//		SetActorLocation(StartLocation); // reset exact loc
//	}
//	else
//	{
//		SetActorLocation(StartLocation + DeltaMovement);
//	}
//}
//
//
//
//void ADynamicPlatform::RotatePlatform(float DeltaTime)
//{
//	RotationAlpha += DeltaTime / PlatformRotateDuration * RotationDirection; //alpha is progress between 0 and 1.
//
//	FRotator DeltaRotation = FRotator(
//		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeRotation.Pitch), RotationAlpha, PlatformRotateCurveEase),
//		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeRotation.Yaw), RotationAlpha, PlatformRotateCurveEase),
//		FMath::InterpEaseInOut(0.0f, float(PlatformRelativeRotation.Roll), RotationAlpha, PlatformRotateCurveEase)
//	);
//
//
//	if (RotationAlpha >= 1.0f)
//	{
//		RotationDirection = -1; // reverse rotation direction
//		SetActorRotation(StartRotation + PlatformRelativeRotation); // reset exact rotation
//	}
//	else if (RotationAlpha <= 0.0f)
//	{
//		RotationDirection = 1; // reverse rotation direction back to original
//		SetActorRotation(StartRotation); // reset exact loc
//	}
//	else
//	{
//		SetActorRotation(StartRotation + DeltaRotation);
//	}
//}