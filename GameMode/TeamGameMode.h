// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlockGameMode.h"
#include "TeamGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ZEROPROTOCOL_API ATeamGameMode : public ABlockGameMode
{
	GENERATED_BODY()
	
public:
	ATeamGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PlayerEliminated(class ABlockCharacter* ElimmedCharacter, 
		class ABlockPlayerController* VictimController,ABlockPlayerController* AttackerController) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
protected:
	virtual void HandleMatchHasStarted() override;
};
