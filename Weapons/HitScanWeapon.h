// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
class UMetaSoundSource;

UCLASS()
class ZEROPROTOCOL_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void FireWeapon(const FVector& HitTarget) override;
	
protected:
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	// ParticleSystem for Impact
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;
	
	//Sound
	UPROPERTY(EditAnywhere)
	UMetaSoundSource* HitSound;
	
private:
	
	// ParticleSystem for Beam through Muzzle
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;
	
	//Sound
	UPROPERTY(EditAnywhere)
	UMetaSoundSource* FireSound;
	
};
