// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

protected:
	/* Handle turning in place variables */
	void TurnInPlace();

	/* Handle calculations for leaning when running */
	void Lean(float DeltaTime);
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	/** The speed of the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	/** Whether or not character is in the air */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	/** Whether or not character is moving */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/* Offset Yaw used for strafing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/* Offset Yaw before we stop moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/* Yaw of the character this frame only updated when standing still and not in air */
	float TIPCharacterYaw;

	/* Yaw of the Character  the previous frame updated when standing still and not in air */
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Turn In Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	/* Rotation curve value this frame */
	float RotationCurve;

	/* Rotation curve value last frame */
	float RotationCurveLastFrame;

	/* The pitch of the aim oration use for aim offset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	/* True when reloading use to prevent aim ofset while reloading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	/* Offset state used to determin which AimOffset to use */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	FRotator CharacterRotation;
	
	FRotator CharacterRotationLastFrame;

	/* Yaw delta used for leaning in the running blendspace */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	float YawDelta;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;
};
