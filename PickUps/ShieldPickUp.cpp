// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickUp.h"
#include "Character/BlockCharacter.h"
#include "CombatComponents/BuffComponent.h"

AShieldPickUp::AShieldPickUp()
{
	bReplicates = true;
}

void AShieldPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(OtherActor);
	
	if (BlockCharacter)
	{
		if (BlockCharacter->GetShield()>= BlockCharacter->GetMaxShield())
		{
			return; 
		}
		UBuffComponent* Buff = BlockCharacter->GetBuff();
		if (Buff)
		{
			Buff->ReplenishShield(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}
	Destroy();
}
