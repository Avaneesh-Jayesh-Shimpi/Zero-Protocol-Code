// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockPlayerState.h"
#include "Character/BlockCharacter.h"
#include "PlayerController/BlockPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlockPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABlockPlayerState, Defeats);
	DOREPLIFETIME(ABlockPlayerState, KilledBy);
	DOREPLIFETIME(ABlockPlayerState, Team);
}


void ABlockPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount); 
	Character = Character == nullptr ? Cast<ABlockCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlockPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore()); 
		}
	}
}

void ABlockPlayerState::AddToDefeats(int32 DefeatAmount)
{
	Defeats += DefeatAmount;
	Character = Character == nullptr ? Cast<ABlockCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlockPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlockPlayerState::MulticastDeathMessage_Implementation(const FString& KillerName)
{
	Character = Character == nullptr ? Cast<ABlockCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlockPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->UpdateDeathMessageHUD(KillerName);
		}
	}
}

void ABlockPlayerState::UpdateDeathMessage(FString KillerName)
{
	MulticastDeathMessage(KillerName);
}

void ABlockPlayerState::UpdateDeathMessageHUD()
{
	Character = Character == nullptr ? Cast<ABlockCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlockPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->UpdateDeathMessageHUD(KilledBy);
		}
	}
}

void ABlockPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	
	Character = Character == nullptr ? Cast<ABlockCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlockPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore()); 
		}
	}
}

void ABlockPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlockCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlockPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats); 
		}
	}
}

void ABlockPlayerState::OnRep_KilledBy()
{
	UpdateDeathMessageHUD();
}

void ABlockPlayerState::OnRep_Team()
{
	ABlockCharacter* BCharacter = Cast<ABlockCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

void ABlockPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
	
	ABlockCharacter* BCharacter = Cast<ABlockCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}
