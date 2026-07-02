// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UParticleSystem;
class UMetaSoundSource;
class UParticleSystemComponent;
class UBoxComponent;

UCLASS()
class ZEROPROTOCOL_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	
	/*
	 * Used With Server-side rewind
	 */
	
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;
	
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 13500;
	
	//Only Set this for Grenades and Rockets
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	
	//Doesn't matter for Grenades and Rockets
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

protected:
	virtual void BeginPlay() override;
	
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDestroyProjectile(bool bPlayerHit);
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	UMetaSoundSource* ImpactSound;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;
	
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;
	
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
	
private:
	
	FTimerHandle DestroyTimer;
	
	UPROPERTY(EditAnywhere)
	float DestroyTime = 4.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;
	
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;
	
};
