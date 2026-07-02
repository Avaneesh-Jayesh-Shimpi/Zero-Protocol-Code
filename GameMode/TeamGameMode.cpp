// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamGameMode.h"
#include "GameState/BlockGameState.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/BlockPlayerController.h"
#include "PlayerState/BlockPlayerState.h"

ATeamGameMode::ATeamGameMode()
{
	bTeamsMatch = true;
}

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	ABlockGameState* BGameState = Cast<ABlockGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		ABlockPlayerState* BPState = NewPlayer->GetPlayerState<ABlockPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamGameMode::PlayerEliminated(class ABlockCharacter* ElimmedCharacter,
	class ABlockPlayerController* VictimController, ABlockPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
	
	ABlockGameState* BGameState = Cast<ABlockGameState>(UGameplayStatics::GetGameState(this));
	ABlockPlayerState* AttackerPState = AttackerController ? Cast<ABlockPlayerState>(AttackerController->PlayerState) : nullptr;
	
	if (BGameState && AttackerPState)
	{
		if (AttackerPState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		else
		{
			BGameState->RedTeamScores();
		}
	}
}

void ATeamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	ABlockGameState* BGameState = Cast<ABlockGameState>(UGameplayStatics::GetGameState(this));
	ABlockPlayerState* BPState = Exiting->GetPlayerState<ABlockPlayerState>();
	
	if (BGameState && BPState)
	{
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}
}

float ATeamGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
 	ABlockPlayerState* AttackerPState = Attacker->GetPlayerState<ABlockPlayerState>();
	ABlockPlayerState* VictimPState = Victim->GetPlayerState<ABlockPlayerState>();
	
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	
	return BaseDamage;
}


void ATeamGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	
	ABlockGameState* BGameState = Cast<ABlockGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			ABlockPlayerState* BPState = Cast<ABlockPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}
