// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;

	void ShowPickupWidget(bool bShowWidget);
	// If projectile is silent, no ammo will be expended, and there'll be no sound or muzzle flash (used for multishot weapons)
	virtual void Fire(const FVector& HitTarget, bool bSilentFire);

	void Drop();

	void AddAmmo(int32 AmmoToAdd);

	void SetHUDAmmo();

	bool IsAmmoEmpty();
	int32 GetMagCapacity();

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShakeBase> CameraShake = nullptr;

	/**
	* Enable/disable custom depth (for weapon outline)
	*/
	void EnableCustomDepth(bool bEnable);

	// Hide weapon (for when emoting)
	void Hide(bool bShouldHideWeapon);


	void SetIsWeaponHidden(bool _bWeaponHidden);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	class ABlasterCharacter* BlasterOwnerCharacter = nullptr;
	class ABlasterPlayerController* BlasterOwnerController = nullptr;


private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_Ammo)
	int32 Ammo;

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UPROPERTY(EditAnywhere)
	int32 ProjectileAmountPerFire = 1;

	UFUNCTION()
	void OnRep_Ammo();

	void ServerSpendAmmo();

	/*
	* Textures for the weapon crosshairs
	*/
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	/**
	* Zoomed FOV while aiming
	*/
	UPROPERTY(EditAnywhere, Category = Zoom)
	float ZoomedFOVMultiplier = 0.5f;
	UPROPERTY(EditAnywhere, Category = Zoom)
	float ZoomInterpSpeed = 20.0f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.2f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;
	UPROPERTY(EditAnywhere, Category = Combat)
	// How much to increase crosshair while holding this weapon (one-time)
	float CrosshairScatterFactor = 0.58f;
	UPROPERTY(EditAnywhere, Category = Combat)
	// How much to increase crosshair while aiming (one-time)
	float CrosshairAimingFactor = -0.58f;
	UPROPERTY(EditAnywhere, Category = Combat)
	// By how much to increase the crosshairs with EACH shot
	float CrosshairShootingFactor = 0.6f;

	UPROPERTY(ReplicatedUsing = OnIsWeaponHiddenReplicated)
	bool bWeaponHidden = false;


	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlashParticles;

	UFUNCTION()
	void OnIsWeaponHiddenReplicated();

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UTexture2D* GetCrosshairsCenter() const { return CrosshairsCenter; }
	FORCEINLINE UTexture2D* GetCrosshairsLeft() const { return CrosshairsLeft; }
	FORCEINLINE UTexture2D* GetCrosshairsRight() const { return CrosshairsRight; }
	FORCEINLINE UTexture2D* GetCrosshairsTop() const { return CrosshairsTop; }
	FORCEINLINE UTexture2D* GetCrosshairsBottom() const { return CrosshairsBottom; }

	FORCEINLINE float GetZoomedFOVMultiplier() const { return ZoomedFOVMultiplier; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetCrosshairScatterFactor() const { return CrosshairScatterFactor; }
	FORCEINLINE float GetCrosshairAimingFactor() const { return CrosshairAimingFactor; }
	FORCEINLINE float GetCrosshairShootingFactor() const { return CrosshairShootingFactor; }

	FORCEINLINE float GetFireDelay() const { return FireDelay; }
	FORCEINLINE float GetAutomatic() const { return bAutomatic; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE int32 GetProjectileAmountPerFire() const { return ProjectileAmountPerFire; }

};


