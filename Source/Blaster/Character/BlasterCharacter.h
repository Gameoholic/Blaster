// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"

#include "BlasterCharacter.generated.h"

// Only temporary player stats should be stored here (persistent only when player isn't dead, resets when dead)
UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	virtual void OnRep_ReplicatedMovement() override;

	void ServerKill();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastKill();
	// Poll for any relevent classes and initialize HUD
	void PollInitHUD();

	class ABlasterPlayerState* BlasterPlayerState = nullptr;

	void ClientRequestDynamicPlatformStates();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayCameraShake(EWeaponType WeaponType);

protected:
	virtual void BeginPlay() override;

	void UpdateHUDHealth();

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
	UFUNCTION(BlueprintCallable)
	void ForceEquipWeapon(AWeapon* WeaponToEquip);
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); //RPC

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/**
	* Animation montages
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraHideThreshold = 200.0f;
	void PlayHitReactMontage();

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	float CalculateSpeed();

	/*
	Player health
	We do this here and not in player state because player state net update is slower
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.0f;
	float PreviousHealth = Health;

	UFUNCTION()
	void OnRep_Health();
	class ABlasterPlayerController* BlasterPlayerController = nullptr;

	bool bKilled = false;

	FTimerHandle RespawnTimer;

	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.0f;

	void ServerRespawnTimerFinished();


	/*
	Dissolve Effect
	*/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	//Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();


	/* 
	* Sync server time for dynamic platforms:
	*/

	bool bClientTest = false;
	UFUNCTION(Server, Reliable)
	void NewServerRequestDynamicPlatformStates();

	UFUNCTION(Server, Reliable)
	void ServerRequestDynamicPlatformStates();

	UFUNCTION(Client, Reliable)
	void ClientReceiveDynamicPlatformStates(const TArray<FDynamicPlatformReplicationData>& DynamicPlatformsReplicationDataArray);
	bool bClientRequestedPlatformStatesFromServer = false;
	float ClientTimeToReceiveResponseToPlatformStatesRequest = 0.0f;

	class UOverheadWidget* OverheadWidgetCasted = nullptr;
	bool bOverheadWidgetDisplayNameSet = false;



	class ALobbyGameState* LobbyGameState = nullptr;
	

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	//replication only happens when the variable changes, not every tick.

	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsKilled() const { return bKilled; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;

};
