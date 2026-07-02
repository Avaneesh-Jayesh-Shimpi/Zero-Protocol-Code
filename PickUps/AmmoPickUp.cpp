// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickUp.h"
#include "Character/BlockCharacter.h"
#include "CombatComponents/CombatComponent.h"

void AAmmoPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(OtherActor);
	if (BlockCharacter)
	{
		UCombatComponent* Combat = BlockCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickUpAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
