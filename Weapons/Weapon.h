// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponType/WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "EquippedSecondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	
	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class ZEROPROTOCOL_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDAmmo();	
	void ShowPickupWidget(bool bShowWidget);
	virtual void FireWeapon(const FVector& HitTarget);
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);
	
	FVector TraceEndWithScatter(const FVector& HitTarget);
	
	//WeaponIcon Texture
	UPROPERTY(EditAnywhere, Category = Combat)
	UTexture2D* WeaponIcon;
	
	/*
	 * Textures for the Weapon CrossHairs
	 */
	UPROPERTY(EditAnywhere, Category = CrossHairs)
	class UTexture2D* CrosshairsCenter;
	
	UPROPERTY(EditAnywhere, Category = CrossHairs)
	UTexture2D* CrosshairsLeft;
	
	UPROPERTY(EditAnywhere, Category = CrossHairs)
	UTexture2D* CrosshairsRight;
	
	UPROPERTY(EditAnywhere, Category = CrossHairs)
	UTexture2D* CrosshairsTop;
	
	UPROPERTY(EditAnywhere, Category = CrossHairs)
	UTexture2D* CrosshairsBottom;
	
	/*
	 * Zoomed FOV while aiming
	 */
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;
	
	UPROPERTY(EditAnywhere)
	float ZoomedInterpSpeed = 20.f;
	
	/*
	 * AutoMatic Fire
	 */
	
	UPROPERTY(EditAnywhere, Category=Combat)
	float FireDelay = 0.15f;
	
	UPROPERTY(EditAnywhere, Category=Combat)
	bool bAutomatic = true;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	class UMetaSoundSource* EquipSound;
	
	/*
	 * Enable Or Disable Custom Depth
	 */
	
	void EnableCustomDepth(bool bEnable);
	
	bool bDestroyWeapon = false;
	
	UPROPERTY(EditAnywhere)
	EFireType FireType;
	
	UPROPERTY(EditAnywhere, Category= "Weapon Scatter")
	bool bUseScatter = false;
 	
protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	virtual void OnRep_Owner() override;
	
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
	void OnSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
	);
	
	/*
	 * Trace End With Scatter
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;
	
	UPROPERTY(EditAnywhere)
	float Damage = 10.f;
	
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 12.f;
	
	// Use this to determine if the weapon type supports SSR by default
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bUseServerSideRewindDefault = false;
	
	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;
	
	UPROPERTY()
	class ABlockCharacter* BlockOwnerCharacter;
	
	UPROPERTY()
	class ABlockPlayerController* BlockOwnerController;
	
	UFUNCTION()
	void OnPingToHigh(bool bPingToHigh);

private:
	UPROPERTY(visibleanywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(visibleanywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState ,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(visibleanywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category= "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;
	
	UPROPERTY(EditAnywhere)
	int32 Ammo;
	
	void SpendRound();
	
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	
	// The Number Of unprocessed Server Requests for Ammo.
	//Incremented in SpendRound, Decremented in Client Update Ammo.
	int32 Sequence = 0;
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomedInterpSpeed; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE UTexture2D* GetWeaponIcon() const { return WeaponIcon; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	bool IsEmpty();
	bool IsFull();
};
