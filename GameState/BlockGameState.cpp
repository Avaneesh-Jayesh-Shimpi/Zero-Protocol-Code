// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockGameState.h"
#include "PlayerState/BlockPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/BlockPlayerController.h"

void ABlockGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABlockGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlockGameState, RedTeamScore);
	DOREPLIFETIME(ABlockGameState, BlueTeamScore);
}

void ABlockGameState::UpdateTopScore(ABlockPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlockGameState::RedTeamScores()
{
	++RedTeamScore;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlockPlayerController* BPlayer = Cast<ABlockPlayerController>(*It);
		if (BPlayer)
		{
			BPlayer->SetHUDRedTeamScores(RedTeamScore);
		}
	}
}

void ABlockGameState::BlueTeamScores()
{
	++BlueTeamScore;
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlockPlayerController* BPlayer = Cast<ABlockPlayerController>(*It);
		if (BPlayer)
		{
			BPlayer->SetHUDBlueTeamScores(BlueTeamScore);
		}
	}
}

void ABlockGameState::OnRep_RedTeamScore()
{
	ABlockPlayerController* BPlayer = Cast<ABlockPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScores(RedTeamScore);
	}
}

void ABlockGameState::OnRep_BlueTeamScore()
{
	ABlockPlayerController* BPlayer = Cast<ABlockPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScores(BlueTeamScore);
	}
}
