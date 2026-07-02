// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlockHUD.h"
#include "Weapons/WeaponType/WeaponTypes.h"
#include "BlockTypes/CombatState.h"
#include "CombatComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZEROPROTOCOL_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlockCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapons();
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
	UFUNCTION(BlueprintCallable)
	void FinishSwap();
	
	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();
	
	void FireButtonPressed(bool bPressed);
	
	UFUNCTION(BlueprintCallable)
	void ShotGunShellReload();
	
	void JumpToShotgunEnd();
	
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();
	
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);
	
	void PickUpAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	
	virtual void BeginPlay() override;
	
	bool bLocallyReloading = false;
	
protected:

	void SetAiming(bool bIsAiming);
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION()
	void OnRep_SecondaryWeapon();
	
	void ExecuteFireWeaponLogic();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgunWeapon();
	
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireWeapon(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void TraceUnderCrossHairs(FHitResult& TraceHitResult);
	void SetCrosshairPackage(float DeltaTime);

	void SetHUDCrosshairs(float DeltaTime);
	
	UFUNCTION(Server, Reliable)
	void ServerReload();
	
	void HandleReloadMontage();
	int32 AmountToReload();

	void InitializeCarriedAmmo();
	
	void UpdateAmmoHUD();
	
	void ThrowGrenade();
	
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;
	
	void DroppedEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackPack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);
	
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	
private:
	UPROPERTY()
	ABlockCharacter* Character;
	UPROPERTY()
	class ABlockPlayerController* Controller;
	UPROPERTY()
	ABlockHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon )
	AWeapon* EquippedWeapon;
	
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;
	
	bool bAimButtonPressed = false;
	
	UFUNCTION()
	void OnRep_Aiming();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed = false;
	
	/*
	 * HUD & CrossHairs
	 */
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	
	FVector HitTarget;
	
	FHUDPackage HUDPackage;
		
	/*
	 * Aiming & FOV
	 */
	
	// Field of View when Not Aiming set to camera's base FOV in BeginPlay
	float DefaultFOV;
	
	UPROPERTY(EditAnywhere, Category= Combat)
	float ZoomedFOV = 30.f;
	
	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category= Combat)
	float ZoomedInterpSpeed = 20.f;
	
	
	void InterpFOV(float DeltaTime);
	
	
	/*
	 * Automatic FireTimer
	 */
	
	FTimerHandle FireTimer;
	
	bool bCanFire = true;
	
	void StartFireTimer();
	void FireTimerFinished();
	
	bool CanFire();
	
	//Carried ammo for currently-equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	
	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	TMap<EWeaponType, int32> CarriedAmmoMap;
	
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;
	
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;
	
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;
	
	UPROPERTY(EditAnywhere)
    int32 StartingShotgunAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;
	
	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	
	UFUNCTION()
	void OnRep_CombatState();
	
	void UpdateAmmoValues();
	void UpdateShotGunAmmoValues();
	
	
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;
	
	UFUNCTION()
	void OnRep_Grenades();
	
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;
	
	void UpdateHUDGrenades();
	
public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	bool ShouldSwapWeapons();
};
