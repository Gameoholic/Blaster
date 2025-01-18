#include "CombatComponent.h"
#include "Blaster/Weapons/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound\SoundCue.h"



UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.0f;
	AimWalkSpeed = 450.0f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			BaseFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = BaseFOV;
		}

		if (Character->HasAuthority())
		{
			//ServerInitCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);

		// Set crosshair target to red if target directly in crosshair
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult, false);
		if (HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->GetCrosshairsCenter();
				HUDPackage.CrosshairsLeft = EquippedWeapon->GetCrosshairsLeft();
				HUDPackage.CrosshairsRight = EquippedWeapon->GetCrosshairsRight();
				HUDPackage.CrosshairsTop = EquippedWeapon->GetCrosshairsTop();
				HUDPackage.CrosshairsBottom = EquippedWeapon->GetCrosshairsBottom();
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			// Calculate crosshair spread

			FVector2D WalkSpeedRange(0.0f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.0f, 1.0f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.0f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				float AirTarget = bAiming ? 1.5 : 2.25f;
				
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, AirTarget, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
			}

			if (bAiming)
			{
				if (EquippedWeapon)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, EquippedWeapon->GetCrosshairAimingFactor(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
				}
				CrosshairVelocityFactor /= 1.5F;
			}
			else
			{
				if (EquippedWeapon)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
				}
				//CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 2.0f); // reset crosshair shooting factor faster
			}
			if (EquippedWeapon)
			{
				CrosshairWeaponFactor = EquippedWeapon->GetCrosshairScatterFactor();
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 1.0f);

			HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CrosshairShootingFactor + CrosshairWeaponFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}






void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDWeaponAmmo(EquippedWeapon->GetAmmo(), EquippedWeapon->GetMagCapacity());
	}



	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, Character->GetActorLocation());
	}

	if (EquippedWeapon->IsAmmoEmpty())
	{
		Reload();
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	if (EquippedWeapon == nullptr || 
		CombatState == ECombatState::ECS_Reloading || EquippedWeapon->GetAmmo() >= EquippedWeapon->GetMagCapacity())
	{
		return;
	}
	ServerReload();
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::UpdateAmmoValues()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	int32 ReloadAmount = AmountToReload();

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDWeaponAmmo(EquippedWeapon->GetAmmo(), EquippedWeapon->GetMagCapacity());
	}

	EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr)
	{
		return;
	}
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr)
	{
		return 0;
	}
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	return RoomInMag;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EquipSound, Character->GetActorLocation());

		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDWeaponAmmo(EquippedWeapon->GetAmmo(), EquippedWeapon->GetMagCapacity());
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, BaseFOV * EquippedWeapon->GetZoomedFOVMultiplier(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, BaseFOV, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}


void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->GetFireDelay()
	);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon && EquippedWeapon->GetAutomatic())
	{
		Fire();
	}
	else
	{
		//Character->GetWorldTimerManager().ClearTimer(FireTimer);
	}

	if (EquippedWeapon->IsAmmoEmpty())
	{
		Reload();
	}
}

void UCombatComponent::Fire()
{
	if (!CanFire() || !EquippedWeapon)
	{
		return;
	}
	bCanFire = false;

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult, true);
	ServerFire(HitResult.ImpactPoint, false);
	// If there are other shots, send them "quietly" (without sound, muzzle flash or expending bullets)
	for (int32 i = 0; i < EquippedWeapon->GetProjectileAmountPerFire() - 1; i++)
	{
		TraceUnderCrosshairs(HitResult, true);
		ServerFire(HitResult.ImpactPoint, true);
	}

	CrosshairShootingFactor += EquippedWeapon->GetCrosshairShootingFactor();

	StartFireTimer();
}


void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, bool bSilentFire) // executes on server
{
	MulticastFire(TraceHitTarget, bSilentFire);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget, bool bSilentFire) //called from server, executes on server+clients
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		if (!bSilentFire)
		{
			Character->PlayFireMontage(bAiming);
		}
		EquippedWeapon->Fire(TraceHitTarget, bSilentFire);
	}
}


void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult, bool bApplyCrosshairSpread)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f); // screen space, viewport space
	FVector2D CrosshairSpreadBonus = FVector2D(0.0f, 0.0f);
	if (bApplyCrosshairSpread)
	{
		CrosshairSpreadBonus = FVector2D(
			FMath::FRandRange(-HUDPackage.CrosshairSpread * 16.0f, HUDPackage.CrosshairSpread * 16.0f), // currently BlasterHud.h STATIC NUMBER CROSSHAIR SPREAD MAX 16.0f
			FMath::FRandRange(-HUDPackage.CrosshairSpread * 16.0f, HUDPackage.CrosshairSpread * 16.0f) // currently BlasterHud.h STATIC NUMBER CROSSHAIR SPREAD MAX 16.0f
		);
	}
	FVector2D ShootLocation = CrosshairLocation + CrosshairSpreadBonus;

	FVector ShootWorldPosition;
	FVector ShootWorldDirection;
	bool bScreenToWorldSuccessful = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		ShootLocation,
		ShootWorldPosition,
		ShootWorldDirection
	);

	if (bScreenToWorldSuccessful)
	{
		FVector Start = ShootWorldPosition + Character->GetActorForwardVector();

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += ShootWorldDirection * (DistanceToCharacter + 100.0f); // Start tracing from a bit after our character
		}

		FVector End = Start + ShootWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Pawn);
	}
}




bool UCombatComponent::CanFire()
{
	return (EquippedWeapon != nullptr && bCanFire && !EquippedWeapon->IsAmmoEmpty() && CombatState == ECombatState::ECS_Unoccupied);
}



