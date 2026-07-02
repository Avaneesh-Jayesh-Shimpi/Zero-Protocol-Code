// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickUpSpawnPoint.h"
#include "Weapons/Weapon.h"

AWeaponPickUpSpawnPoint::AWeaponPickUpSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AWeaponPickUpSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority())
	{
		SpawnWeapon();
	}
}

void AWeaponPickUpSpawnPoint::SpawnWeapon()
{
	if (WeaponClasses.Num() <= 0) return;
	int32 Selection = FMath::RandRange(0, WeaponClasses.Num() - 1);
	TSubclassOf<AWeapon> SelectedWeaponClass = WeaponClasses[Selection];
	if (SelectedWeaponClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 2. Spawn the weapon at this actor's location
			World->SpawnActor<AWeapon>(
				SelectedWeaponClass, 
				GetActorLocation(), 
				GetActorRotation(), 
				SpawnParams
			);
		}
	}
	Destroy();
}


void AWeaponPickUpSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

