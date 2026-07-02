// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZEROPROTOCOL_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();
	friend class ABlockCharacter;
	
	//Jump
	void SetInitialJumpVelocity(float Velocity);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	
	//Speed
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	
	//Heal
	void Heal(float HealAmount, float HealingTime);
	
	//Shield
	void ReplenishShield(float ReplenishAmount, float ReplenishTime);
protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	UPROPERTY()
	class ABlockCharacter* Character;
	
	/*
	 * Health Buff
	 */
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;
	
	/*
	 * Shield Replenish
	 */
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float AmountToReplenishShield = 0.f;
	
	/*
	 * SpeedBuff
	 */
	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;
	void ResetSpeeds();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);
	
	/*
	 * JumpBuff
	 */
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);
};
