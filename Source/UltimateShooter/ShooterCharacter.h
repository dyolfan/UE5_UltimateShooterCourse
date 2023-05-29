// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Onocupied"),
	ECS_FireTimeInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),

	EAT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class ULTIMATESHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	/* Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/* Called for forwards/backwards input */
	void MoveForward(float Value);

	/* Called for side to side input */
	void MoveRight(float Value);

	/** 
	* Called via input to turn at a given rate.
	* @param Rate  This is a normalized rate, i.r. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to look up/down at a give rate.
	* @param Rate  This is a normalized rate, i.r. 1.0 means 100% of desired rate
	*/
	void LookUpAtRate(float Rate);

	/**
	* Rotate controller base on mouse X
	* @param Value	The input value from mouse movement
	*/
	void Turn(float Value);

	/**
	* Rotate controller base on mouse Y
	* @param Value	The input value from mouse movement
	*/
	void LookUp(float Value);

	/** Called when a fire button is presed */
	void FireWeapon();

	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	void AimingButtonPressed();

	void AimingButtonReleased();

	void CameraInterpZoom(float DeltaTime);

	// Set BaseTurnRate and BaseLookUpRate based in aiming
	void SetLookRates();

	void CalculateCrosshairSpread(float DeltaTime);

	void StartCrosshairBulletFire();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	void FireButtonPressed();

	void FireButtonReleased();

	void StartFireTimer();
	
	UFUNCTION()
	void AutoFireReset();

	/* Line trace for items under the crosshair */
	bool TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation);

	/* Trace for items if OverlappedItemCount more than zero */
	void TraceForItems();

	/* Spawns default weapon and quipes it */
	class AWeapon* SpawnDefaultWeapon();

	/* Takes a weapon and attaches it to the mesh */
	void EquipWeapon(AWeapon* WeaponToEquip);

	/* Drop weapon and let it fall to the ground */
	void DropWeapon();

	void SelectButtonPressed();

	void SelectButtonReleased();

	/* Drops currently equiped Weapon and Equips TraceHitItem */
	void SwapWeapon(AWeapon* WeaponToSwap);

	/* Initialize the ammo map with ammo values */
	void InitializeAmmoMap();

	/* Check to make sure that our weapon has ammo */
	bool WeaponHasAmmo();

	/* Fire button options */
	void PlayFireSound();
	void SendBullet();
	void PlayGunfireMontage();

	/* Bound to R key and Gamepad Face Button Left*/
	void ReloadButtonPressed();

	/* Handle weapon reload */
	void ReloadWeapon();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/* Checks to see if we have ammo fo the EquipedWeapons ammo type */
	bool CarryingAmmo();

	/* Called from Animation Blueprint with Grab Clip notify */
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	/* Called from Animation Blueprint with Release Clip notify */
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	void CrouchButtonPressed();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Randomized gunshot sound cue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	/* Flash spawned at BarrelSocket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	/* Montage for firing the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	/* Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	/* Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/* Default cammera FOV */
	float CameraDefaultFOV;

	/* Field of view for camera when zoomed */
	float CameraZoomedFOV;

	/* Current FOV this frame */
	float CameraCurrentFOV;

	/* Interp speed of zoomin when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	/* Scale factor for mouse look sensitivity. Turn rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMix = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	/* Scale factor for mouse look sensitivity. Look up rate when not aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMix = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	/* Scale factor for mouse look sensitivity. Turn rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMix = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	/* Scale factor for mouse look sensitivity. Look up rate when aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMix = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	/* True when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/* Turn rate while no aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	/* Look up rate when not aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/* Turn rate when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/* Look up rate when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	/* Determines the spread of the crosshair */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	/* Velocity component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	/* Air component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	/* Aim component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	/* Shooting component for crosshairs spread */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	float ShootTimeDuration;

	bool bFiringBullet;

	FTimerHandle CrosshairShootTimer;

	/* left Mouse button or right console trigger pressed */
	bool bFireButtonPressed;

	/* True whe we can fire. False when waiting for the fire */
	bool bShouldFire;

	/* Rate of automatic gunfire */
	float AutomaticFireRate;

	/* Sets a timer between gunshots */
	FTimerHandle AutoFireTimer;

	/* True if we should trace every frame for items */
	bool bShouldTraceForItems;

	/* Number of overlaped AItems */
	int8 OverlappedItemCount;

	/* The AItem we hit last frame */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "True"))
	class AItem* TraceHitItemLastFrame;

	/* Currently equiped weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	AWeapon* EquipedWeapon;

	/* Set this in blueprint for the default weapon class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	/* The Item crrently hit by trace in TraceForItems (could be null) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	AItem* TraceHitItem;

	/* Distance outward fro the camera interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	float CameraInterpDistance;

	/* Distance upward fro the camera interp destination */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	float CameraInterpElevation;

	/* Map to keep track of ammo of different ammo types */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "True"))
	TMap<EAmmoType, int32> AmmoMap;

	/* Starting ammount of 9mm ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "True"))
	int32 Starting9mmAmmo;

	/* Starting ammount of AR ammo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "True"))
	int32 StartingARAmmo;


	/* Combat state, can only fire or reload if Unoccupied */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	ECombatState CombatState;

	/* Montage for reload animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;

	/* Transform of the clip when we firs grab the clip during reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	FTransform ClipTransform;

	/* Scene component to attach to the Character's hand during reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "True"))
	USceneComponent* HandSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "True"))
	bool bCrouching;
public:
	/** Returns CameraBoom subobject */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Return FollowCamera subobject */
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera;  }

	FORCEINLINE bool GetIsAiming() const { return bAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	/* Adds/subtracts to/from overlapped item count and updates the bShouldTraceForItems */
	void IncrementOverlappedItemCount(int8 Amount);

	FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE bool GetCrounching() const { return bCrouching; }
};
