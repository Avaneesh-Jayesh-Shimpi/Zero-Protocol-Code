// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpSpawnPoint.h"
#include "PickUp.h"

APickUpSpawnPoint::APickUpSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickUpSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickUpTimer((AActor*)nullptr);
}

void APickUpSpawnPoint::SpawnPickUp()
{
	int32 NumPickupsClasses = PickUpClasses.Num();
	if (NumPickupsClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupsClasses - 1);
		SpawnedPickUp = GetWorld()->SpawnActor<APickUp>(PickUpClasses[Selection], GetActorTransform());
		
		if (HasAuthority() && SpawnedPickUp)
		{
			SpawnedPickUp->OnDestroyed.AddDynamic(this, &APickUpSpawnPoint::StartSpawnPickUpTimer);
		}
	}
}

void APickUpSpawnPoint::StartSpawnPickUpTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(SpawnPickUpTimeMin, SpawnPickUpTimeMax);
	GetWorldTimerManager().SetTimer(
		SpawnPickUpTimer,
		this,
		&APickUpSpawnPoint::SpawnPickUpTimerFinished,
		SpawnTime
	);
}

void APickUpSpawnPoint::SpawnPickUpTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickUp();
	}
}

void APickUpSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

