// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockGameMode.h"
#include "Character/BlockCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/BlockPlayerController.h"
#include "PlayerState/BlockPlayerState.h"
#include "GameState/BlockGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlockGameMode::ABlockGameMode()
{
	bDelayedStart = true;
}

void ABlockGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlockGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlockGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlockPlayerController* BlockPlayer = Cast<ABlockPlayerController>(*It);
		if (BlockPlayer)
		{
			BlockPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
	 
}

float ABlockGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void ABlockGameMode::PlayerEliminated(class ABlockCharacter* ElimmedCharacter,
                                      class ABlockPlayerController* VictimController, ABlockPlayerController* AttackerController)
{
	ABlockPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlockPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlockPlayerState* VictimPlayerState = VictimController ? Cast<ABlockPlayerState>(VictimController->PlayerState) : nullptr;
	
	ABlockGameState* BlockGameState = GetGameState<ABlockGameState>();
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlockGameState)
	{
		TArray<ABlockPlayerState*> PlayerCurrentlyInTheLead;
		for (auto LeadPlayer : BlockGameState->TopScoringPlayers)
		{
			PlayerCurrentlyInTheLead.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		BlockGameState->UpdateTopScore(AttackerPlayerState);
		
		if (BlockGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlockCharacter* Leader = Cast<ABlockCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}
		
		for (int32 i = 0; i < PlayerCurrentlyInTheLead.Num(); i++)
		{
			if (!BlockGameState->TopScoringPlayers.Contains(PlayerCurrentlyInTheLead[i]))
			{
				ABlockCharacter* Loser = Cast<ABlockCharacter>(PlayerCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (AttackerPlayerState && VictimPlayerState)
	{
		FString PlayerName = AttackerPlayerState->GetPlayerName();
		VictimPlayerState->UpdateDeathMessage(PlayerName);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlockPlayerController* BlockPlayer = Cast<ABlockPlayerController>(*It);
		if (BlockPlayer && AttackerPlayerState && VictimPlayerState)
		{
			BlockPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ABlockGameMode::RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ABlockGameMode::PlayerLeftGame(ABlockPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ABlockGameState* BlockGameState = GetGameState<ABlockGameState>();
	if (BlockGameState && BlockGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlockGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ABlockCharacter* CharacterLeaving = Cast<ABlockCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}
