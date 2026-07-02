// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Character/BlockCharacter.h"
#include "PlayerController/BlockPlayerController.h"
#include "CombatComponents/LagCompensationComponent.h"
#include "MetasoundSource.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

void AShotGun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::FireWeapon(FVector());
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	
	// This will be null on simulated proxies
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	
	// Removed InstigatorController from this check
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		
		// Maps hit Character to number of times hit
		TMap<ABlockCharacter*, uint32> HitMap;
		TMap<ABlockCharacter*, uint32> HeadShotHitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);
			
			ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(FireHit.GetActor());
			if (BlockCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				
				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(BlockCharacter)) HeadShotHitMap[BlockCharacter]++;
					else HeadShotHitMap.Emplace(BlockCharacter, 1);
				}
				else
				{
					if (HitMap.Contains(BlockCharacter)) HitMap[BlockCharacter]++;
					else HitMap.Emplace(BlockCharacter, 1);
				}
				
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}
				
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this, 
						HitSound, FireHit.ImpactPoint, 
						0.5f, 
						FMath::FRandRange(-0.5f, 0.5f)
					);
				}
			}
		}
		TArray<ABlockCharacter*> HitCharacters;
		
		// Maps Character Hit To total Damage
		TMap<ABlockCharacter*, float> DamageMap;
		
		//Calculate body shot damage by multiplying times hit x Damage - store in damageMap
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		
		//Calculate HeadShot damage by multiplying times hit x Headshot Damage - store in damageMap
		for (auto HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
				
				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}
		
		// Loop through DamageMap to get total damage for each character
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				// Check if we are the authority AND (we are not using SSR OR we are locally controlling this pawn)
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
					DamagePair.Key, // Character that was hit
					DamagePair.Value,  // Damage calculated in the two for loops above
					InstigatorController,
				this,
					UDamageType::StaticClass()
					);
				}
		
			}
		}
		
		
		if (!HasAuthority() && bUseServerSideRewind)
		{
			BlockOwnerCharacter = BlockOwnerCharacter == nullptr ? Cast<ABlockCharacter>(OwnerPawn) : BlockOwnerCharacter;
			BlockOwnerController = BlockOwnerController == nullptr ? Cast<ABlockPlayerController>(InstigatorController) : BlockOwnerController;
			if (BlockOwnerController && BlockOwnerCharacter && BlockOwnerCharacter->GetLagCompensation() && BlockOwnerCharacter->IsLocallyControlled())
			{
				BlockOwnerCharacter->GetLagCompensation()->ShotGunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					BlockOwnerController->GetServerTime() - BlockOwnerController->SingleTripTime
				);
			}
		}
	}
}

void AShotGun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		
		HitTargets.Add(ToEndLoc);
	}
}
