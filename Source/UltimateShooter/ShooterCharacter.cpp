// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "DrawDebugHelpers.h"
#include "Item.h"
#include "Weapon.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	// Base rates for turning/looking up
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	// Camera field of view values
	CameraDefaultFOV(0.f),  // Set in BeginPlay
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
	// Mouse look sensitivity scale factors
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.5f),
	MouseAimingLookUpRate(0.5f),
	// truw when aiming the weapon
	bAiming(false),
	// Turn rates for Aiming/not aiming
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	// Bullet fire timer variables
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	// Automatic fire variables
	bFireButtonPressed(false),
	bShouldFire(true),
	AutomaticFireRate(0.1f),
	// Item trace variables
	bShouldTraceForItems(false),
	// Camera interp location variables
	CameraInterpDistance(250.f),
	CameraInterpElevation(75.f),
	Starting9mmAmmo(120),
	StartingARAmmo(85),
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f;   // The cammera follow at this distance behind character
	CameraBoom->bUsePawnControlRotation = true;  // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);  // Attach camera to end ob Boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Dont rotate when the controller rotates. Let the controller ony affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // .. at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create HandSceneComponent
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>("HandSceneComp");
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if(FollowCamera) 
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// Spawn the default weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());

	InitializeAmmoMap();
}

void AShooterCharacter::MoveForward(float Value)
{
	if((Controller != nullptr) &&  (Value != 0.0f)) 
	{
		// findout which way is forward
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// findout which way is right
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// Calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if(bAiming)
	{ 
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookScaleFactor{};
	if (bAiming)
	{
		LookScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (EquipedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (WeaponHasAmmo()) {
		PlayFireSound();
		SendBullet();
		PlayGunfireMontage();
		EquipedWeapon->DecrementAmmo();

		StartFireTimer();
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{

	/* Check for crosshair trace hit */
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		// Tentative beam location - still need to trace from gun
		OutBeamLocation == CrosshairHitResult.Location;
	}
	else // no crosshair trace hit
	{
		// Out beam location is the end location for the line trace
	}

	// Perform a second trace, this time from a gun barrel
	FHitResult WeapoTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(
		WeapoTraceHit,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility
	);
	if (WeapoTraceHit.bBlockingHit) // object between barrel and BeamEndPoint?
	{
		OutBeamLocation = WeapoTraceHit.Location;
		return true;
	}
	return false;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
	GetFollowCamera()->SetFieldOfView(CameraZoomedFOV);
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
	GetFollowCamera()->SetFieldOfView(CameraDefaultFOV);
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	// Set current camera FOV
	if (bAiming)
	{
		// Interpolate to Zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		// Interpolate to Default FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelicytyMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange, 
		VelicytyMultiplierRange, 
		Velocity.Size());

	// Calculate crosshair in Air factor
	if(GetCharacterMovement()->IsFalling()) // is in air?
	{ 
		// Spread the crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	} 
	else // Character is on the ground
	{
		// Shrint the crosshairs rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	// Calcualte crosshair aim factor
	if (bAiming) // Are we aiming?
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.7f, DeltaTime, 30.f);
	}
	else // Not aiming
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	// True 0.05 seconds after firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.5f, DeltaTime, 60.f);
	} 
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 5.f);
	}

	CrosshairSpreadMultiplier = 
		0.5f +
		CrosshairVelocityFactor +
		CrosshairInAirFactor -
		CrosshairAimFactor
		+ CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimeInProgress;

	GetWorldTimerManager()
		.SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if(bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else {
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	// Get Viewport Size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	// CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		// Trace from crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems) 
	{
		FHitResult ItemTraceResult;
		FVector OutHitLocation;
		TraceUnderCrosshairs(ItemTraceResult, OutHitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				// Show Item's pickup Widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
			}

			// We hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// We are hitting a different AItem this frame from last frame
					// Or AItem is null.
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}

			// Store a reference to HitItem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	else if (TraceHitItemLastFrame)
	{
		// No longer overlapping any items,
		// Item last frame should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check subclass variable
	if (DefaultWeaponClass)
	{
		// Spawn Weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip) {
		

		// Get right hand socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			// Attach the weapon to the socket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}
		EquipedWeapon = WeaponToEquip;
		EquipedWeapon->SetItemState(EItemState::EIS_Equiped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquipedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);

		EquipedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquipedWeapon->SetItemState(EItemState::EIS_Falling);
		EquipedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if(TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);

		if (TraceHitItem->GetPickupSound()) 
		{
			UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickupSound());
		}
	}
}

void AShooterCharacter::SelectButtonReleased()
{
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquipedWeapon == nullptr) return false;
	return EquipedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquipedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquipedWeapon->GetItemMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd) {
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					BeamEnd
				);
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				BeamParticles,
				SocketTransform
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}

	StartCrosshairBulletFire();
}

void AShooterCharacter::PlayGunfireMontage()
{
	// Play Hip Fire Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (EquipedWeapon == nullptr) return;

	// Do we have ammo of the correct type
	if(CarryingAmmo() && !EquipedWeapon->ClipIsFull()) 
	{
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquipedWeapon->GetReloadMontageSection());
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	// Update the combat state
	CombatState = ECombatState::ECS_Unoccupied;


	if (EquipedWeapon == nullptr) return;
	// Update AmmoMap
	const auto AmmoType{ EquipedWeapon->GetAmmoType() };
	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];

		const int32 MagEmptySpace = EquipedWeapon->GetMagazineCapacity() - EquipedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// Reload the magazine with all the ammo we are carrying
			EquipedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
		} 
		else
		{
			// Fill the magazine
			EquipedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
		}
		AmmoMap.Add(AmmoType, CarriedAmmo);
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquipedWeapon == nullptr) return false;

	auto AmmoType = EquipedWeapon->GetAmmoType();
	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if (EquipedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;
	
	// Index for the clip bone on equiped weapon
	int32 ClipBoneIndex{ EquipedWeapon->GetItemMesh()->GetBoneIndex(EquipedWeapon->GetClipBoneName()) };
	// Store the transform of the clip
	ClipTransform = EquipedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquipedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquipedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle zoom interpolation when character is aiming
	CameraInterpZoom(DeltaTime);
	// Change look sensivity based on aiming
	SetLookRates();
	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);
	// Check OverlappedItemCount, then trace for items
	TraceForItems();
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", EInputEvent::IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", EInputEvent::IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", EInputEvent::IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", EInputEvent::IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", EInputEvent::IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };

	// Desired = CameraWorldLocation + Forward * A + Up * B
	return CameraWorldLocation + CameraForward * CameraInterpDistance
		+ FVector(0.f, 0.f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon) {
		SwapWeapon(Weapon);
		if (Item->GetEquipSound())
		{
			UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
		}
	}
}

