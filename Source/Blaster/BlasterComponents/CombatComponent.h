#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapons/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f


class AWeapon; // forward declaration


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter; // gives full access for private and protected
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool IsAiming);

	UFUNCTION()

	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget); //to server

	UFUNCTION(NetMulticast, Reliable) // to all clients+server
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	int32 AmountToReload();


private:
	class ABlasterCharacter* Character = nullptr;
	class ABlasterPlayerController* Controller = nullptr;
	class ABlasterHUD* HUD = nullptr;

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/// HUD and Crosshairs
	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;


	/**
	* Aiming and FOV
	*/
	float BaseFOV; // fov when not aiming, set to camera's base fov in BeginPlay

	float CurrentFOV;

	void InterpFOV(float DeltaTime);

	/*
	Automatic Fire
	*/

	FTimerHandle FireTimer;

	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
	
	bool CanFire();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
	void UpdateAmmoValues();
};
