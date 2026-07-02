// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Character/BlockCharacter.h"
#include "MetasoundSource.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/WeaponType/WeaponTypes.h"
#include "Particles/ParticleSystemComponent.h"
#include "CombatComponents/LagCompensationComponent.h"
#include "PlayerController/BlockPlayerController.h"

#include "DrawDebugHelpers.h"

void AHitScanWeapon::FireWeapon(const FVector& HitTarget)
{
	Super::FireWeapon(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	
	// This will be null on simulated proxies, which is fine!
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	
	// Removed InstigatorController from this check
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		
		ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(FireHit.GetActor());
		
		// Apply damage only if we have authority and a valid controller
		if (BlockCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;				
				
				UGameplayStatics::ApplyDamage(
                				BlockCharacter,
                				DamageToCause,
                				InstigatorController,
                				this,
                				UDamageType::StaticClass()
                			);
			}
			if (!HasAuthority() && bUseServerSideRewind)
			{
				BlockOwnerCharacter = BlockOwnerCharacter == nullptr ? Cast<ABlockCharacter>(OwnerPawn) : BlockOwnerCharacter;
				BlockOwnerController = BlockOwnerController == nullptr ? Cast<ABlockPlayerController>(InstigatorController) : BlockOwnerController;
				if (BlockOwnerController && BlockOwnerCharacter && BlockOwnerCharacter->GetLagCompensation() && BlockOwnerCharacter->IsLocallyControlled())
				{
					BlockOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						BlockCharacter,
						Start,
						HitTarget,
						BlockOwnerController->GetServerTime() - BlockOwnerController->SingleTripTime);
				}
			}
		}
		
		// All visuals and audio below will now execute for all clients
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
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		
		World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECC_Visibility);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}
		
		//DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, true);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

