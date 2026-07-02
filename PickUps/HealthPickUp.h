// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "HealthPickUp.generated.h"

UCLASS()
class ZEROPROTOCOL_API AHealthPickUp : public APickUp
{
	GENERATED_BODY()
	
public:
	AHealthPickUp();
	
protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult
		);
	
private:
	UPROPERTY(EditAnywhere, Category = "Healing")
	float MinHealAmount = 25.f;
	
	UPROPERTY(EditAnywhere, Category = "Healing")
	float MaxHealAmount = 75.f;
	
	UPROPERTY(EditAnywhere, Category = "Healing")
	float HealingTime = 5.f;
};
