// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"


void AProjectileWeapon::FireWeapon(const FVector& HitTarget)
{
	Super::FireWeapon(HitTarget);
		
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//From muzzle Flash Socket to Hit Location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator  TargetRotation = ToTarget.Rotation();
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		
		AProjectile* SpawnProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority())//Server
			{
				if (InstigatorPawn->IsLocallyControlled())//Server,Host - use replicated Projectile
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(),TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = false;
					SpawnProjectile->Damage = Damage;
					SpawnProjectile->HeadShotDamage = HeadShotDamage;
				}
				else//Server, not Locally Controlled - spawn non-replicated projectile, SSR
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(),TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = true;
				}
			}
			else//Client, Using SSR
			{
				if (InstigatorPawn->IsLocallyControlled())//Client Locally Controlled - spawn non-replicated projectile, use SSR
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(),TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = true;
					SpawnProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnProjectile->InitialVelocity = SpawnProjectile->GetActorForwardVector() * SpawnProjectile->InitialSpeed;
				}
				else//Client Not Locally Controlled - spawn non-replicated projectile, no SSR
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(),TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else// Weapon Not Using SSR
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(),TargetRotation, SpawnParams);
				SpawnProjectile->bUseServerSideRewind = false;
				SpawnProjectile->Damage = Damage;
				SpawnProjectile->HeadShotDamage = HeadShotDamage;
			}
		}
	}
}
