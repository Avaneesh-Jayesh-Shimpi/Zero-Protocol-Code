// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpSpawnPoint.generated.h"

UCLASS()
class ZEROPROTOCOL_API APickUpSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
	APickUpSpawnPoint();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickUp>> PickUpClasses;
	
	UPROPERTY()
	APickUp* SpawnedPickUp;
	
	void SpawnPickUp();
	void SpawnPickUpTimerFinished();
	UFUNCTION()
	void StartSpawnPickUpTimer(AActor* DestroyedActor);
private:
	FTimerHandle SpawnPickUpTimer;	
	
	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimeMin;
	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimeMax;
public:
	

};
