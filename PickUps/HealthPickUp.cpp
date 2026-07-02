// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickUp.h"
#include "Character/BlockCharacter.h"
#include "CombatComponents/BuffComponent.h"

AHealthPickUp::AHealthPickUp()
{
	bReplicates = true;
}


void AHealthPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(OtherActor);
	if (BlockCharacter)
	{
		// 1. Check if the player is already at full health
		if (BlockCharacter->GetHealth() >= BlockCharacter->GetMaxHealth())
		{
			// Do nothing and return early so the pickup isn't destroyed
			return; 
		}
		UBuffComponent* Buff = BlockCharacter->GetBuff();
		if (Buff)
		{
			// 2. Generate the random heal amount for this specific pickup event
			float RandomHealAmount = FMath::RandRange(MinHealAmount, MaxHealAmount);
			
			// 3. Apply the random amount
			Buff->Heal(RandomHealAmount, HealingTime);
		}
	}
	// 4. Move Destroy() inside the if-statement so it only gets destroyed 
	// if a valid character actually consumes the healing.
	Destroy();
}
