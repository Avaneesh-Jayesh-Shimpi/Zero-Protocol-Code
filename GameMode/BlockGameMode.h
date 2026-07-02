// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlockGameMode.generated.h"

namespace  MatchState
{
	extern ZEROPROTOCOL_API const FName Cooldown; // Match Duration has been Reached, Display Winner and begin  cooldown Timer
}

/**
 * 
 */
UCLASS()
class ZEROPROTOCOL_API ABlockGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABlockGameMode();
	virtual void Tick( float DeltaTime ) override;
	virtual void PlayerEliminated(class ABlockCharacter* ElimmedCharacter, 
		class ABlockPlayerController* VictimController,ABlockPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(class ABlockPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	
	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 15.f;
	
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	
	float LevelStartingTime = 0.f;
	
	bool bTeamsMatch = false;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	float CountDownTime = 0.f;
	
public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }
};
