// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlockTypes/Team.h"
#include "BlockPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ZEROPROTOCOL_API ABlockPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	/*
	 * Replication Notification
	 */
	virtual void OnRep_Score() override;
	
	UFUNCTION()
	virtual void OnRep_Defeats();
	
	UFUNCTION()
	void OnRep_KilledBy();
	
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatAmount);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDeathMessage(const FString& KillerName);
	void UpdateDeathMessage(FString KillerName);
private:
	UPROPERTY()
	class ABlockCharacter* Character;
	UPROPERTY()
	class ABlockPlayerController* Controller;
	
	
	void UpdateDeathMessageHUD();
	
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
	
	UPROPERTY(ReplicatedUsing = OnRep_KilledBy)
	FString KilledBy;
	
	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;
	
	UFUNCTION()
	void OnRep_Team();
	
public:
	
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
};
