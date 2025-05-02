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
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Blaster.h"



UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.0f;
	AimWalkSpeed = 450.0f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, MainWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
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
		// Will only turn red if hit actor is blaster character, AND the hit component was the mesh (not the capsule component for example).
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
			if (MainWeapon)
			{
				HUDPackage.CrosshairsCenter = MainWeapon->GetCrosshairsCenter();
				HUDPackage.CrosshairsLeft = MainWeapon->GetCrosshairsLeft();
				HUDPackage.CrosshairsRight = MainWeapon->GetCrosshairsRight();
				HUDPackage.CrosshairsTop = MainWeapon->GetCrosshairsTop();
				HUDPackage.CrosshairsBottom = MainWeapon->GetCrosshairsBottom();
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
				if (MainWeapon)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, MainWeapon->GetCrosshairAimingFactor(), DeltaTime, MainWeapon->GetZoomInterpSpeed());
				}
				CrosshairVelocityFactor /= 1.5F;
			}
			else
			{
				if (MainWeapon)
				{
					CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, MainWeapon->GetZoomInterpSpeed());
				}
				//CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 2.0f); // reset crosshair shooting factor faster
			}
			if (MainWeapon)
			{
				CrosshairWeaponFactor = MainWeapon->GetCrosshairScatterFactor();
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 1.0f);

			HUDPackage.CrosshairSpread = CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairAimFactor + CrosshairShootingFactor + CrosshairWeaponFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}


void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}





void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip, bool bForceMainWeaponSlot)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	if (bForceMainWeaponSlot)
	{
		// If both weapon slots are taken, replace main weapon. Otherwise, make current main weapon the secondary
		if (MainWeapon && SecondaryWeapon)
		{
			MainWeapon->Drop();
		}
		else if (MainWeapon)
		{
			SecondaryWeapon = MainWeapon;
			SecondaryWeapon->SetIsWeaponHidden(true);
		}

		MainWeapon = WeaponToEquip;
		MainWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	}
	else
	{
		if (MainWeapon && SecondaryWeapon)
		{
			MainWeapon->Drop();
			MainWeapon = WeaponToEquip;
			MainWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		}
		else if (MainWeapon)
		{
			SecondaryWeapon = WeaponToEquip;
			SecondaryWeapon->SetIsWeaponHidden(true);
		}
		else
		{
			MainWeapon = WeaponToEquip;
			MainWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		}
	}

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(MainWeapon, Character->GetMesh());
	}
	WeaponToEquip->SetOwner(Character);

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		if (MainWeapon)
		{
			Controller->SendChatMessage(FName("Client hi: 1"));
		}
		else
		{
			Controller->SendChatMessage(FName("Client hi: 1 no weapon"));
		}
		Controller->SetHUDWeaponAmmo(MainWeapon->GetAmmo(), MainWeapon->GetMagCapacity());
		Controller->SetHUDMainWeapon(MainWeapon->GetDisplayName(), MainWeapon->GetIcon());
		if (SecondaryWeapon)
		{
			Controller->SetHUDSecondaryWeapon(SecondaryWeapon->GetDisplayName(), SecondaryWeapon->GetIcon());
		}
	}



	if (WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation());
	}

	if (MainWeapon->IsAmmoEmpty())
	{
		Reload();
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipWeaponAtSlot(AWeapon* WeaponToEquip, bool bMainWeaponSlot)
{
	if (WeaponToEquip == nullptr)
	{
		return;
	}
	if (bMainWeaponSlot)
	{
		if (MainWeapon)
		{
			MainWeapon->Drop();
			MainWeapon = nullptr;
		}
		EquipWeapon(WeaponToEquip, false);
	}
	else
	{
		if (SecondaryWeapon)
		{
			SecondaryWeapon->Drop();
			SecondaryWeapon = nullptr;
		}
		EquipWeapon(WeaponToEquip, false);
	}
}

void UCombatComponent::SwitchWeaponButtonReleased()
{
	if (!SecondaryWeapon)
	{
		return;
	}

	// Client side fix for reload animation not restarting when switching between 2 reloading weapons
	if (CombatState == ECombatState::ECS_Reloading)
	{
		Character->ResetReloadMontage();
	}
	ServerSwitchWeapon();
}

void UCombatComponent::ServerSwitchWeapon_Implementation()
{
	if (!SecondaryWeapon)
	{
		return;
	}
	// Switch weapons
	AWeapon* PreviousMainWeapon = MainWeapon;
	MainWeapon = SecondaryWeapon;
	SecondaryWeapon = PreviousMainWeapon;

	// Update the meshes
	SecondaryWeapon->SetIsWeaponHidden(true);
	MainWeapon->SetIsWeaponHidden(false);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(MainWeapon, Character->GetMesh());
	}

	if (Character->IsLocallyControlled())
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDMainWeapon(MainWeapon->GetDisplayName(), MainWeapon->GetIcon());
			if (SecondaryWeapon)
			{
				Controller->SetHUDSecondaryWeapon(SecondaryWeapon->GetDisplayName(), SecondaryWeapon->GetIcon());
			}
			Controller->SetHUDWeaponAmmo(MainWeapon->GetAmmo(), MainWeapon->GetMagCapacity());
		}

		if (MainWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, MainWeapon->EquipSound, Character->GetActorLocation());
		}
	}

	if (CombatState == ECombatState::ECS_Reloading)
	{
		StopReload();
	}

	if (MainWeapon->IsAmmoEmpty())
	{
		ServerReload();
	}
	StopFireTimer();
}

void UCombatComponent::Reload()
{
	if (MainWeapon == nullptr ||
		CombatState == ECombatState::ECS_Reloading || MainWeapon->GetAmmo() >= MainWeapon->GetMagCapacity())
	{
		return;
	}
	MainWeapon->SetIsWeaponHidden(false);
	ServerReload();
}

void UCombatComponent::StopReload()
{
	Character->StopReloadMontage();
	CombatState = ECombatState::ECS_Unoccupied;
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || MainWeapon == nullptr)
	{
		return;
	}

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
	MainWeapon->SetIsWeaponHidden(false);
}

void UCombatComponent::UpdateAmmoValues()
{
	if (MainWeapon == nullptr)
	{
		return;
	}
	int32 ReloadAmount = AmountToReload();

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDWeaponAmmo(MainWeapon->GetAmmo(), MainWeapon->GetMagCapacity());
	}

	MainWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || MainWeapon == nullptr)
	{
		return;
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDWeaponAmmo(MainWeapon->GetAmmo(), MainWeapon->GetMagCapacity());
	}

	MainWeapon->AddAmmo(-1);
	bCanFire = true;

	if (MainWeapon->GetAmmo() == MainWeapon->GetMagCapacity()) // If full
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	//Jump to ShotgunEnd section
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
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
		StopReload();
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
	if (MainWeapon == nullptr)
	{
		return 0;
	}
	int32 RoomInMag = MainWeapon->GetMagCapacity() - MainWeapon->GetAmmo();
	return RoomInMag;
}

void UCombatComponent::OnRep_MainWeaponEquipped()
{
	if (!Character)
	{
		return;
	}
	if (MainWeapon)
	{
		if (SecondaryWeapon)
		{
			SecondaryWeapon->SetIsWeaponHidden(true);
		}
		MainWeapon->SetIsWeaponHidden(false);

		MainWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(MainWeapon, Character->GetMesh());
		}
		if (Character->IsLocallyControlled())
		{
			UE_LOG(LogTemp, Warning, TEXT("GOT REPD sex %d HAS ASUTHORITY"), Character->Controller);
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		UGameplayStatics::PlaySoundAtLocation(this, MainWeapon->EquipSound, Character->GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("GOT REPD sex %d"), Character->Controller);

		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDWeaponAmmo(MainWeapon->GetAmmo(), MainWeapon->GetMagCapacity());
			Controller->SetHUDMainWeapon(MainWeapon->GetDisplayName(), MainWeapon->GetIcon());
			if (SecondaryWeapon)
			{
				Controller->SetHUDSecondaryWeapon(SecondaryWeapon->GetDisplayName(), SecondaryWeapon->GetIcon());
			}
		}

		// Fix on clients for switching from reloading weapon to another reloading weapon
		if (CombatState == ECombatState::ECS_Reloading)
		{
			Character->ResetReloadMontage();
		}
	}
	else
	{
		MainWeapon->Destroy();
		//const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		//if (HandSocket)
		//{
		//	HandSocket->AttachActor(nullptr, Character->GetMesh());
		//}
	}
}

void UCombatComponent::OnRep_SecondaryWeaponEquipped()
{
	
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (MainWeapon == nullptr) return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, BaseFOV * MainWeapon->GetZoomedFOVMultiplier(), DeltaTime, MainWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, BaseFOV, DeltaTime, MainWeapon->GetZoomInterpSpeed());
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || MainWeapon == nullptr)
	{
		return;
	}
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && MainWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
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
	if (MainWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer, this, &UCombatComponent::FireTimerFinished, MainWeapon->GetFireDelay()
	);
}

void UCombatComponent::StopFireTimer()
{
	Character->GetWorldTimerManager().ClearTimer(FireTimer);
	FireTimerFinished();
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if (bFireButtonPressed && MainWeapon && MainWeapon->GetAutomatic())
	{
		Fire();
	}
	else
	{
		//Character->GetWorldTimerManager().ClearTimer(FireTimer);
	}

	if (MainWeapon->IsAmmoEmpty())
	{
		Reload();
	}
}

void UCombatComponent::Fire()
{
	if (!CanFire() || !MainWeapon)
	{
		return;
	}
	bCanFire = false;

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult, true);
	ServerFire(HitResult.ImpactPoint, false);
	// If there are other shots, send them "quietly" (without sound, muzzle flash or expending bullets)
	for (int32 i = 0; i < MainWeapon->GetProjectileAmountPerFire() - 1; i++)
	{
		TraceUnderCrosshairs(HitResult, true);
		ServerFire(HitResult.ImpactPoint, true);
	}

	CrosshairShootingFactor += MainWeapon->GetCrosshairShootingFactor();

	// Play camera shake
	UWorld* World = GetWorld();
	if (MainWeapon->CameraShake != nullptr && World)
	{
		UGameplayStatics::PlayWorldCameraShake(
			World,
			MainWeapon->CameraShake,
			Character->GetActorLocation(),
			0.0f,
			500.0f
		);
	}

	StartFireTimer();
}


void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, bool bSilentFire) // executes on server
{
	MulticastFire(TraceHitTarget, bSilentFire);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget, bool bSilentFire) //called from server, executes on server+clients
{
	if (MainWeapon == nullptr || Character == nullptr)
	{
		return;
	}

	if ((CombatState == ECombatState::ECS_Reloading && MainWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
			|| (CombatState == ECombatState::ECS_Unoccupied))
	{
		if (!bSilentFire)
		{
			Character->PlayFireMontage(bAiming);
		}
		MainWeapon->Fire(TraceHitTarget, bSilentFire);
		CombatState = ECombatState::ECS_Unoccupied;
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

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Pawn); // This means anything that any actor that has Pawn set to Block in the collision presets counts. To disable switch to ignore
	}
}






bool UCombatComponent::CanFire()
{
	if (MainWeapon == nullptr || !bCanFire || MainWeapon->IsAmmoEmpty())
	{
		return false;
	}
	if (MainWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && CombatState == ECombatState::ECS_Reloading)
	{
		return true;
	}
	return (CombatState == ECombatState::ECS_Unoccupied);
}



