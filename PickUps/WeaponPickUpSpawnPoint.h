// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponPickUpSpawnPoint.generated.h"

class AWeapon;

UCLASS()
class ZEROPROTOCOL_API AWeaponPickUpSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
	AWeaponPickUpSpawnPoint();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Spawning")
	TArray<TSubclassOf<AWeapon>> WeaponClasses;

	void SpawnWeapon();

private:
	
	
public:
	

};
