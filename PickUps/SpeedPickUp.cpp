// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickUp.h"
#include "Character/BlockCharacter.h"
#include "CombatComponents/BuffComponent.h"

void ASpeedPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(OtherActor);
	if (BlockCharacter)
	{
		UBuffComponent* Buff = BlockCharacter->GetBuff();
		if (Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff,CrouchSpeedBuff,SpeedBuffTime);
		}
	}
	Destroy();
}
