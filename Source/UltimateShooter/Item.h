// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),
	EIR_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equiped UMETA(DisplayName = "Equiped"),
	EIS_Falling UMETA(DisplayName = "Falling"),
	
	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class ULTIMATESHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Called when overlapping AreaSphere */
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherCompo, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);

	/* Called when end overlapping AreaSphere */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/* Sets active starts based on rarity */
	void SetActiveStars();

	/* Sets properties of the Item's component base on State */
	void SetItemProperties(EItemState State);

	/* Called when ItemInterpTimer is finished */
	void FinishInterping();

	/* Handle item interpolation when in the EquipInterping state */
	void ItemInterp(float DeltaTime);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/* Skeletal mesh for the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	/* Line trace collides with box to show HUD widgets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	/* Popup widget for when the player looks at the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	/* Enables item tracing when overlaped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	/* The name which appears on the Pickup Widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	/* Item count (ammo, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	/* Item rarity - determins number of stars in Pickup Widget */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	/* State if the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	/* The curve asset to use for item's Z location when interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	class UCurveFloat* ItemZCurve;

	/* Starting location when interping begins */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	FVector ItemInterpStartLocation;

	/* Target interp location infront of the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	FVector CameraTargetLocation;

	/* True when interping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	bool bInterping;

	/* Plays when we start interping */
	FTimerHandle ItemInterpTimer;

	/* Pinter to the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	class AShooterCharacter* Character;

	/* Duration of the curve and timer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	float ZCurveTime;

	/* X and Y for the Item while interpin in the EquipedInterping state */
	float ItemInterpX;
	float ItemInterpY;

	/* Initial Yaw offset between the camera and the interping item */
	double InterpInitialYawOffset;

	/* Curve used to scale item when interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	UCurveFloat* ItemScaleCurve;

	/* Should played when Item is picked up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	class USoundCue* PickupSound;

	/* Sound played when Item is equiped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item properties", meta = (AllowPrivateAccess = "True"))
	class USoundCue* EquipSound;
public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; };
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }

	void SetItemState(EItemState NewState);

	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }

	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }

	/* Called from AShooter character class */
	void StartItemCurve(AShooterCharacter* Char);
};
