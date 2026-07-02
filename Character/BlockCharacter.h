// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "BlockTypes/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "BlockTypes/CombatState.h"
#include "BlockTypes/Team.h"
#include "BlockCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;	
class UBoxComponent;
class ULagCompensationComponent;

UCLASS()
class ZEROPROTOCOL_API ABlockCharacter : public ACharacter , public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlockCharacter();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	
	// Montages
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();
	//
	virtual void OnRep_ReplicatedMovement() override;
	void DropOrDestroy(AWeapon* Weapon);
	void DropOrDestroyWeapon();
	virtual void Destroyed() override;
	
	void Elim(bool bPlayerLeftGame);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(Replicated)
	bool bDisableGamePlay = false;
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	
	void SpawnDefaultWeapon();
	
	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;
	
	bool bFinishSwapping = false;
	
	/*Callbacks for Input*/
	virtual void Jump() override;
	
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();
	
	FOnLeftGame OnLeftGame;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();
	
	void SetTeamColor(ETeam Team);
	
protected:
	void RotateInPlace(float DeltaTime);
	UPROPERTY(EditAnywhere, Category= Input)
	UInputMappingContext* CharacterContext;
	
	UPROPERTY(EditAnywhere, Category= Input)
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category= Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category= Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category= Input)
	UInputAction* EKeyAction;
	
	UPROPERTY(EditAnywhere, Category= Input)
	UInputAction* SwapKeyAction;

	UPROPERTY(EditAnywhere, Category= Input)
	UInputAction* CKeyAction;
	
	UPROPERTY(EditAnywhere, Category= Input)
	UInputAction* GKeyAction;

	UPROPERTY(EditAnywhere, Category= Input)
	UInputAction* AimKeyAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* FireAction;
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* ReloadAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void AimKeyPressed(const FInputActionValue& Value);
	void GrenadeButtonPressed();
	void FireButtonPressed(const FInputActionValue& Value);
	void FireButtonReleased();
	void ReloadButtonPressed();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	void SwapWeapon();
	
	//Hit Particles
	void SpawnHitParticles(const FVector_NetQuantize& ImpactLocation, const FVector_NetQuantizeNormal& ImpactNormal);
	
	/*Callbacks for Input*/
	void EKeyPressed();
	void CrouchKeyPressed();
	
	//Montage
	void PlayHitReactMontage();
	
	//spawn Point
	void SetSpawnPoint();
	
	//
	void OnPlayerStateInitialized();
	
	//Damage
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	//Poll For Any Relevant Classes & initialize our HUD
	void PollInit();
	
	/*
	 * Hit Boxes used for Server-Side Rewind
	 */
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* ViewCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	class UParticleSystem* HitImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Combat")
	class UMetaSoundSource* HitImpactSound;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	/*
	 * Block Components
	 */
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;
	
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;
	
	UPROPERTY(VisibleAnywhere)
	ULagCompensationComponent* LagCompensation;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	
	UFUNCTION(Server, Reliable)
	void ServerSwapWeapon();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/*
	 * Animation Montages
	 */
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;
	
	//End Montage section
	
	void HideCharacterIfCameraClose();
	
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();
	
	/*
	 * Player Health
	 */
	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere,Category="Player Stats")
	float Health = 100.f;
	
	UFUNCTION()
	void OnRep_Health(float LastHealth);
	
	/*
	 * Player Shield
	 */
	UPROPERTY(EditAnywhere, Category="Player Stats")
	float MaxShield = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere,Category="Player Stats")
	float Shield = 0.f;
	
	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	UPROPERTY()
	class ABlockPlayerController* BlockPlayerController;
	
	bool bElimmed = false;
	
	FTimerHandle ElimTimer;
	
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	
	void ElimTimerFinished();
	
	bool bLeftGame = false;
	
	/*
	 * Dissolve Effect
	 */
	
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	
	FOnTimelineFloat DissolveTrack;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();
	
	//Dynamic Instance that we can change at runtime 
	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;
	
	//Material Instance set on the Blueprint, used with the Dynamic Material Instance
	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstance* DissolveMaterialInstance;
	
	/*
	 * Team Colors
	 */
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedDissolveMatInst;
	
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedMaterial;
	
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueDissolveMatInst;
	
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueMaterial;
	
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DefaultMaterial;
	
	
	UPROPERTY()
	class ABlockPlayerState* BlockPlayerState;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;
	
	UPROPERTY()
	class UNiagaraComponent* CrownComponent;
	
	/*
	 * Grenade
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;
	
	/*
	 * Default Weapon
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
	UPROPERTY()
	class ABlockGameMode* BlockGameMode;
	
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FVector GetHitTarget() const;
	AWeapon* GetEquippedWeapon();
	ECombatState GetCombatState() const;
	//Health
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	//Shield
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	//
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch; }
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool GetDisableGameplay() const { return  bDisableGamePlay; } 
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return ViewCamera; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	bool IsLocallyReloading();
};
