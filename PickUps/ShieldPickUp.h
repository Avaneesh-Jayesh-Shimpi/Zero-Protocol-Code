// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "ShieldPickUp.generated.h"

/**
 * 
 */
UCLASS()
class ZEROPROTOCOL_API AShieldPickUp : public APickUp
{
	GENERATED_BODY()
	
public:
	AShieldPickUp();
	
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
	UPROPERTY(EditAnywhere, Category = "ShieldReplenish")
	float ShieldReplenishAmount = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "ShieldReplenish")
	float ShieldReplenishTime = 5.f;
};
