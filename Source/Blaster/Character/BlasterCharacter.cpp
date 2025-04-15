// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapons/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Platform/DynamicPlatform.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Weapons/WeaponTypes.h"
#include "Blaster/HUD/OverheadWidget.h"
#include "Blaster/MultiplayerSessions/MultiplayerSessionsSubsystem.h"
#include "Blaster/GameInstance/BlasterGameInstance.h"
#include "Blaster/GameMode/LobbyGameMode.h"
#include "Blaster/GameState/LobbyGameState.h"
#include "Blaster/HUD/EmoteWheel/EmoteWheel.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Blaster/Items/Item.h"
#include "Blaster/HUD/BlasterScrollBox.h"
#include "Blaster/HUD/Chat.h"
#include "Blaster/Shop/ShopVolume.h"


// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true); //component replication doesn't need to be registered

	EmoteAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EmoteAudio"));
	EmoteAudio->SetupAttachment(RootComponent);
	EmoteAudio->bAutoActivate = false;

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 500.0f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, SelectedEmoteIndex);
}


void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.0f;
}


void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABlasterCharacter::OnComponentBeginOverlap);
		GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ABlasterCharacter::OnComponentEndOverlap);
	}

	if (!HasAuthority() && IsLocallyControlled())
	{
		NewServerRequestDynamicPlatformStates();
	}

	LobbyGameState = Cast<ALobbyGameState>(UGameplayStatics::GetGameState(this));
}

void ABlasterCharacter::NewServerRequestDynamicPlatformStates_Implementation()
{
	TArray<AActor*> DynPlatformActors;
	UGameplayStatics::GetAllActorsOfClass(this, ADynamicPlatform::StaticClass(), DynPlatformActors);
	for (AActor* DynPlatformActor : DynPlatformActors)
	{
		ADynamicPlatform* DynPlatform = Cast<ADynamicPlatform>(DynPlatformActor);
		if (DynPlatform)
		{
			DynPlatform->ServerReplicatePlatformData();
		}
	}
}


void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.1f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCameraIfCharacterClose();
	PollInitHUD();

	if (!HasAuthority() && IsLocallyControlled() && bClientRequestedPlatformStatesFromServer)
	{
		ClientTimeToReceiveResponseToPlatformStatesRequest += DeltaTime;
	}

	if (HasAuthority())
	{
		AActor* Act = GetCharacterMovement()->CurrentFloor.HitResult.GetActor();
		if (Act)
		{ // bruh we really gonna do this shit every tick? maybe just use a collision box after all?
			if (Cast<ADynamicPlatform>(Act))
			{
				//Cast<ADynamicPlatform>(Act)->bActive = !Cast<ADynamicPlatform>(Act)->bActive;
			}
		}
	}



	// Set overhead widget displayname (not loaded immediately on BeginPlay on clients)
	if (!bOverheadWidgetDisplayNameSet) // Only show nametags of other players, not self
	{
		OverheadWidgetCasted = OverheadWidgetCasted == nullptr ? Cast<UOverheadWidget>(OverheadWidget->GetUserWidgetObject()) : OverheadWidgetCasted;
		if (GetPlayerState() && OverheadWidgetCasted && !IsLocallyControlled() && LobbyGameState)
		{
			OverheadWidgetCasted->SetDisplayText(GetPlayerState()->GetPlayerName());
			// If player is host, set host icon
			if (LobbyGameState->HostPlayerName == GetPlayerState()->GetPlayerName())
			{
				OverheadWidgetCasted->SetIsHost();
			}
			bOverheadWidgetDisplayNameSet = true;
		}
	}

	// Make overhead widget face the locally controlled player character
	FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(
		OverheadWidget->GetComponentLocation(),
		UGameplayStatics::GetPlayerCameraManager(this, 0)->GetRootComponent()->GetComponentLocation()
	);
	OverheadWidget->SetWorldRotation(NewRotation);

	
	
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &ABlasterCharacter::SwitchWeaponButtonReleased);
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Emote", IE_Pressed, this, &ABlasterCharacter::EmoteButtonPressed);
	PlayerInputComponent->BindAction("Emote", IE_Released, this, &ABlasterCharacter::EmoteButtonReleased);
	PlayerInputComponent->BindAction("Chat", IE_Pressed, this, &ABlasterCharacter::ChatButtonPressed);
	PlayerInputComponent->BindAction("Shop", IE_Pressed, this, &ABlasterCharacter::ShopButtonPressed);
	PlayerInputComponent->BindAxis("EmoteWheelPage", this, &ABlasterCharacter::ChangeEmoteWheelPage);
	PlayerInputComponent->BindAxis("ScrollBoxMouseWheel", this, &ABlasterCharacter::ScrollBoxMouseWheel);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::PollInitHUD()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			// Display current stats
			BlasterPlayerState->DisplayUpdatedScore();
			BlasterPlayerState->DisplayUpdatedKills();
			BlasterPlayerState->DisplayUpdatedDeaths();
		}
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->MainWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->MainWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->MainWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Sniper:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ResetReloadMontage()
{
	if (Combat == nullptr || Combat->MainWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_SetPosition(ReloadMontage, 0.0f);
	}
}

void ABlasterCharacter::StopReloadMontage()
{
	if (Combat == nullptr || Combat->MainWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Stop(0.3, ReloadMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->MainWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value); // speed in character movement component
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.0f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value); // speed in character movement component
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed(); // send server rpc
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::EmoteButtonPressed()
{	
	if (!EmoteWheelWidget)
	{
		return;
	}
	EmoteWheel = CreateWidget<UEmoteWheel>(BlasterPlayerController, EmoteWheelWidget);
	EmoteWheel->AddToViewport();

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetWidgetToFocus(EmoteWheel->TakeWidget());
	BlasterPlayerController->SetInputMode(InputModeData);
	BlasterPlayerController->SetShowMouseCursor(true);

	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		BlasterPlayerController->SetMouseLocation(ViewportSize.X / 2, ViewportSize.Y / 2);
	}
}

void ABlasterCharacter::EmoteButtonReleased()
{
	if (!EmoteWheel)
	{
		return;
	}
	EmoteWheel->ReleaseEmoteWheel();

	FInputModeGameOnly InputModeData;
	BlasterPlayerController->SetInputMode(InputModeData);
	BlasterPlayerController->SetShowMouseCursor(false);
}

void ABlasterCharacter::ChangeEmoteWheelPage(float MouseWheelDirection)
{
	if (!EmoteWheel)
	{
		return;
	}

	if (MouseWheelDirection == 1.0f)
	{
		EmoteWheel->OnEmoteWheelNextPagePressed();
	}
	else if (MouseWheelDirection == -1.0f)
	{
		EmoteWheel->OnEmoteWheelPreviousPagePressed();
	}
}

void ABlasterCharacter::ScrollBoxMouseWheel(float MouseWheelDirection)
{
	if (!FocusedBlasterScrollBox)
	{
		return;
	}
	if (MouseWheelDirection == 0.0f)
	{
		return;
	}
	FocusedBlasterScrollBox->HandleMouseWheelScroll(MouseWheelDirection);
}

void ABlasterCharacter::ChatButtonPressed()
{
	if (!BlasterPlayerController)
	{
		return;
	}
	BlasterPlayerController->ToggleChat();
}

void ABlasterCharacter::ShopButtonPressed()
{
	if (!BlasterPlayerController || !bCanOpenShop)
	{
		return;
	}
	// todo: open shop here
}


// RPC that runs on server
void ABlasterCharacter::ServerSendPlayerChatMessage_Implementation(FName Message, FName SendingPlayerName)
{
	if (!GetWorld())
	{
		return;
	}
	FString NewMessage = SendingPlayerName.ToString();
	NewMessage.Append(": ");
	NewMessage.Append(Message.ToString());
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		BlasterPlayer->ClientSendMessage(FName(NewMessage));
	}
}

void ABlasterCharacter::OnChatMessageReceived(FName Message)
{
	BlasterGameInstance = BlasterGameInstance == nullptr ? Cast<UBlasterGameInstance>(GetGameInstance()) : BlasterGameInstance;
	if (BlasterGameInstance)
	{
		BlasterGameInstance->AddChatMessage(Message);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->MainWeapon == nullptr) return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.0f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.0f || bIsInAir) // running or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.0f && !IsLocallyControlled()) // Fix Pitch in Multiplayer (#59)
	{
		// Map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.0f, 360.0f);
		FVector2D OutRange(-90.0f, 0.0f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
		//todo: same solution to our yaw problem?
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	// handle turning for simulated proxies
	if (Combat == nullptr || Combat->MainWeapon == nullptr)
	{
		return;
	}

	bRotateRootBone = false;

	float Speed = CalculateSpeed();
	if (Speed > 0.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::SwitchWeaponButtonReleased()
{
	if (Combat)
	{
		Combat->SwitchWeaponButtonReleased();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

// this will only be called on server (authority)
void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	PreviousHealth = Health;
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth); // replicated to clients, see OnRep_Health
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.0f)
	{
		SetSelectedEmoteIndex(-1);
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			if (BlasterPlayerController && AttackerController)
			{
				BlasterGameMode->PlayerKilled(this, BlasterPlayerController, AttackerController);
			}
		}

		// todo: this sohudl called on locally controlled killed player
		// If killed in lobby, player will be kicked
		ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
		if (LobbyGameMode)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Controller);
			if (PlayerController)
			{
				LobbyGameMode->KickPlayer(PlayerController);
			}
		}
	}
}

void ABlasterCharacter::ForceEquipWeapon(AWeapon* WeaponToEquip)
{
	if (Combat)
	{
		Combat->EquipWeapon(WeaponToEquip);
	}
}

void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PreviousHealth = Health;
	PlayHitReactMontage();
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.0f, DeltaTime, 4.0f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.0f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}




void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) // last value before replication happened
{
	// If weapon stopped overlapping
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(OverlappingWeapon != nullptr);
	}
	else
	{
		OverlappingWeapon->ShowPickupWidget(OverlappingWeapon != nullptr);
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation() // SERVER RPC
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}



void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraHideThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->MainWeapon && Combat->MainWeapon->GetWeaponMesh())
		{
			Combat->MainWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->MainWeapon && Combat->MainWeapon->GetWeaponMesh())
		{
			Combat->MainWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.Size();
}


void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(PreviousHealth, Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::ClientRequestDynamicPlatformStates()
{
	//ServerRequestDynamicPlatformStates();
	//bClientRequestedPlatformStatesFromServer = true;
}
void ABlasterCharacter::ServerRequestDynamicPlatformStates_Implementation()
{
	//UE_LOG(LogTemp, Warning, TEXT("received request from client."));

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[server] received request from client")));
	//TArray<AActor*> DynPlatformActors;
	//UGameplayStatics::GetAllActorsOfClass(this, ADynamicPlatform::StaticClass(), DynPlatformActors);

	//// Get all state data of existing dynamic platforms necessary for replication (rpc'd) down to client
	//TArray<FDynamicPlatformReplicationData> DynamicPlatformsReplicationDataArray;
	//for (AActor* DynPlatformActor : DynPlatformActors)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("bMR BEAST actor"));

	//	ADynamicPlatform* DynPlatform = Cast<ADynamicPlatform>(DynPlatformActor);
	//	if (DynPlatform)
	//	{
	//		FDynamicPlatformReplicationData DynamicPlatformReplicationData;
	//		DynamicPlatformReplicationData.Id = DynPlatform->GetId();
	//		UE_LOG(LogTemp, Warning, TEXT("blastercahracter ID IS SEX: %d, %d"), DynPlatform->GetId(), DynPlatform->GetUniqueID());
	//		DynamicPlatformReplicationData.CurrentChangeTime = DynPlatform->GetCurrentChangeTime();
	//		DynamicPlatformReplicationData.bOppositeDirection = DynPlatform->GetIsOppositeDirection();
	//		DynamicPlatformReplicationData.bActive = DynPlatform->GetIsActive();


	//		UE_LOG(LogTemp, Warning, TEXT("SERVER JIMMY id is: %d"), DynamicPlatformReplicationData.Id);

	//		DynamicPlatformsReplicationDataArray.Add(DynamicPlatformReplicationData);
	//	}
	//}

	//ClientReceiveDynamicPlatformStates(DynamicPlatformsReplicationDataArray);
}

void ABlasterCharacter::ClientReceiveDynamicPlatformStates_Implementation(const TArray<FDynamicPlatformReplicationData>& DynamicPlatformsReplicationDataArray)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[client] received response from server")));
	//bClientRequestedPlatformStatesFromServer = false;

	//TArray<AActor*> DynPlatformActors;
	//UGameplayStatics::GetAllActorsOfClass(this, ADynamicPlatform::StaticClass(), DynPlatformActors);

	//// Find matching dynamic platforms between server and client by Id's
	//for (AActor* DynPlatformActor : DynPlatformActors)
	//{

	//	ADynamicPlatform* DynPlatform = Cast<ADynamicPlatform>(DynPlatformActor);
	//	if (DynPlatform)
	//	{
	//		for (FDynamicPlatformReplicationData DynamicPlatformsReplicationData : DynamicPlatformsReplicationDataArray)
	//		{

	//			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[client] id: %d"), DynamicPlatformsReplicationData.Id));

	//			UE_LOG(LogTemp, Warning, TEXT("[client] Trying to match: %d, %d"), DynamicPlatformsReplicationData.Id, DynPlatform->GetId());

	//			if (DynamicPlatformsReplicationData.Id == DynPlatform->GetId())
	//			{
	//				UE_LOG(LogTemp, Warning, TEXT("[client] found match for id: %d"), DynPlatform->GetId());

	//				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString::Printf(TEXT("[client] found match")));

	//				DynPlatform->ClientUpdateDynamicPlatformState(
	//					DynamicPlatformsReplicationData, 
	//					ClientTimeToReceiveResponseToPlatformStatesRequest
	//				);
	//			}
	//		}
	//	}
	//}
}

void ABlasterCharacter::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA<AShopVolume>())
	{
		OnEnterShopVolume();
	}
}

void ABlasterCharacter::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA<AShopVolume>())
	{
		OnLeaveShopVolume();
	}
}

void ABlasterCharacter::OnEnterShopVolume()
{
	bCanOpenShop = true;

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShowShopIcon(true);
	}
}

void ABlasterCharacter::OnLeaveShopVolume()
{
	bCanOpenShop = false;

	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShowShopIcon(false);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled() && OverlappingWeapon) // properties are not replicated from server to server.
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

// Called only on server players who just reached 0 hp
void ABlasterCharacter::ServerKill()
{
	MulticastKill();
	GetWorldTimerManager().SetTimer(
		RespawnTimer,
		this,
		&ABlasterCharacter::ServerRespawnTimerFinished,
		RespawnDelay
	);
	if (Combat && Combat->MainWeapon)
	{
		Combat->MainWeapon->Drop();
	}
}

// called on server+clients when just reached 0 hp
void ABlasterCharacter::MulticastKill_Implementation()
{
	if (BlasterPlayerController)
	{
		//BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bKilled = true;
	// Enable ragdoll physics:
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); // default is query only
	GetMesh()->SetSimulatePhysics(true);

	// Dissolve mesh effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 250.0f);
	}
	StartDissolve();

	// Disable movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hide sniper scope
	if (IsLocallyControlled() && Combat && Combat->bAiming && Combat->MainWeapon && Combat->MainWeapon->GetWeaponType() == EWeaponType::EWT_Sniper)
	{
		ShowSniperScopeWidget(false);
	}
}

void ABlasterCharacter::ServerRespawnTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RespawnPlayer(this, Controller);
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->MainWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr)
	{
		return nullptr;
	}
	return Combat->MainWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr)
	{
		return FVector();
	}
	FHitResult HitResult;
	Combat->TraceUnderCrosshairs(HitResult, false);
	return HitResult.ImpactPoint;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combat == nullptr)
	{
		return ECombatState::ECS_MAX;
	}
	return Combat->CombatState;
}


// EMOTES

// Method called on client/server when selected emote is updated
void ABlasterCharacter::SetSelectedEmoteIndex(int32 _SelectedEmoteIndex)
{
	// First, update emote immediately on client
	if (!HasAuthority())
	{
		SelectedEmoteIndex = _SelectedEmoteIndex;
		UpdateEmote();
	}
	// Send RPC to server, OR update emote immediately if on server
	ServerPlayEmote(_SelectedEmoteIndex); // Play emote on server
}

// Called on server when a client's emote changes
void ABlasterCharacter::ServerPlayEmote_Implementation(int32 _SelectedEmoteIndex)
{
	SelectedEmoteIndex = _SelectedEmoteIndex; // This will get replicated to clients
	UpdateEmote();
}

// Called on clients
void ABlasterCharacter::OnRep_SelectedEmoteIndex()
{
	UpdateEmote();
}


// Called on client/server when emote index was changed and emote needs to play/stop. Handles sound, animation and weapon
void ABlasterCharacter::UpdateEmote()
{
	// Animation:
	// EmoteIndex automatically sets animation in BlasterAnimInstance.cpp
	
	// Hide weapon:
	Combat->MainWeapon->SetIsWeaponHidden(SelectedEmoteIndex != -1);

	// Sound:
	if (SelectedEmoteIndex != -1)
	{
		BlasterGameInstance = BlasterGameInstance == nullptr ? Cast<UBlasterGameInstance>(GetGameInstance()) : BlasterGameInstance;
		if (BlasterGameInstance)
		{
			USoundCue* EmoteSound = BlasterGameInstance->Emotes[SelectedEmoteIndex].Sound; // Because emote wheel grabs emotes directly from game instance, can safely assume the exact emote we need exists in the array
			EmoteAudio->SetSound(EmoteSound);
			EmoteAudio->Play();
		}
	}
	else
	{
		EmoteAudio->Stop();
	}
}



int32 ABlasterCharacter::GetSelectedEmoteIndex()
{
	return SelectedEmoteIndex;
}


// ITEMS
void ABlasterCharacter::AddItem(UItem* Item)
{
	if (Item1 && Item2)
	{
		return;
	}
	Item1 == nullptr ? Item1 = Item : Item2 = Item;
	if (BlasterPlayerController)
	{
		if (Item1 != nullptr)
		{
			BlasterPlayerController->SetHUDItem1(Item1->GetName(), Item1->GetIcon());
		}
		if (Item2 != nullptr)
		{
			BlasterPlayerController->SetHUDItem2(Item2->GetName(), Item2->GetIcon());
		}
	}
}

void ABlasterCharacter::RequestBlasterScrollBoxFocus(UBlasterScrollBox* ScrollBox, bool bFocus)
{
	// If the currently focused scroll box is not the one that requested focus, ignore the request (happens when it destructs but is not focused)
	if (FocusedBlasterScrollBox != nullptr && ScrollBox != FocusedBlasterScrollBox)
	{
		return;
	}
	// If the scrollbox is already focused, ignore the request (because the request happens every tick when the scrollbox is being hovered)
	if (FocusedBlasterScrollBox == ScrollBox && bFocus)
	{
		return;
	}

	// Focus / unfocus
	if (FocusedBlasterScrollBox != nullptr)
	{
		FocusedBlasterScrollBox->OnUnfocus();
	}
	if (bFocus)
	{
		FocusedBlasterScrollBox = ScrollBox;
		FocusedBlasterScrollBox->OnFocus();
	}
	else
	{
		FocusedBlasterScrollBox = nullptr;
	}
}
