// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockPlayerController.h"

#include "EnhancedInputComponent.h"
#include "BlockTypes/Announements.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/BlockHUD.h"
#include "Character/BlockCharacter.h"
#include "Components/Image.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/BlockGameMode.h"
#include "HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "CombatComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameState/BlockGameState.h"
#include "PlayerState/BlockPlayerState.h"
#include "Components/Image.h"
#include "HUD/ReturnToMainMenu.h"
#include "BlockTypes/Announements.h"


void ABlockPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlockPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
		if (BlockHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				BlockHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				BlockHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				BlockHUD->AddElimAnnouncement("You", "YourSelf");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				BlockHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Themselves");
				return;
			}
			BlockHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}


void ABlockPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Started, this, &ABlockPlayerController::ShowReturnToMainMenu);
	}
}

void ABlockPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	BlockHUD = Cast<ABlockHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlockPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABlockPlayerController, MatchState);
	DOREPLIFETIME(ABlockPlayerController, bShowTeamsScores);
}

void ABlockPlayerController::HideTeamScores()
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->RedTeamScore  &&
				BlockHUD->CharacterOverlay->BlueTeamScore &&
					BlockHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		BlockHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		BlockHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
		BlockHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
	}
}

void ABlockPlayerController::InitTeamScores()
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->RedTeamScore  &&
				BlockHUD->CharacterOverlay->BlueTeamScore &&
					BlockHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		BlockHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		BlockHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
		BlockHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
	}
}

void ABlockPlayerController::SetHUDRedTeamScores(int32 RedScore)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->RedTeamScore;
	
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		BlockHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ABlockPlayerController::SetHUDBlueTeamScores(int32 BlueScore)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->BlueTeamScore;
	
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		BlockHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void ABlockPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamsScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

void ABlockPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}


void ABlockPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		if (PlayerState == nullptr)
		{
			PlayerState = GetPlayerState<APlayerState>();
		}
		if (PlayerState)
		{
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold) // ping is compressed; it's actually ping / 4
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying =
		BlockHUD && BlockHUD->CharacterOverlay && 
			BlockHUD->CharacterOverlay->HighPingAnimation &&
				BlockHUD->CharacterOverlay->IsAnimationPlaying(BlockHUD->CharacterOverlay->HighPingAnimation);	
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime; 
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void ABlockPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		
		// Grab the character reference once
		ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(GetPawn());

		if (bReturnToMainMenuOpen)
		{
			// --- WIDGET IS ACTIVE: Menu Setup & Disable Movement ---
			ReturnToMainMenu->MenuSetup();

			if (BlockCharacter && BlockCharacter->GetCharacterMovement())
			{
				if (BlockCharacter->GetCharacterMovement())
				{
					BlockCharacter->GetCharacterMovement()->StopMovementImmediately();
				}
				BlockCharacter->bDisableGamePlay = true;
				
				if (BlockCharacter->GetCombat())
				{
					BlockCharacter->GetCombat()->FireButtonPressed(false);
				}
			}
		}
		else
		{
			// --- WIDGET IS INACTIVE: Menu TearDown & Enable Movement ---
			ReturnToMainMenu->MenuTearDown();

			if (BlockCharacter && BlockCharacter->GetCharacterMovement())
			{
				// Set bDisableGamePlay back to false so custom input works again
				BlockCharacter->bDisableGamePlay = false;
			}
		}
	}
}

// Is the Ping too high?
void ABlockPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABlockPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlockPlayerController::HighPingWarning()
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->HighPingImage &&
				BlockHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		BlockHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlockHUD->CharacterOverlay->PlayAnimation(
			BlockHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5
			);
	}
}

void ABlockPlayerController::StopHighPingWarning()
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->HighPingImage &&
				BlockHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		BlockHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (BlockHUD->CharacterOverlay->IsAnimationPlaying(BlockHUD->CharacterOverlay->HighPingAnimation))
		{
			BlockHUD->CharacterOverlay->StopAnimation(BlockHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void ABlockPlayerController::ServerCheckMatchState_Implementation()
{
	if (ABlockGameMode* GameMode = Cast<ABlockGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		WarmUpTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmUpTime, MatchTime, CooldownTime,LevelStartingTime);
	}
}

void ABlockPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float WarmUp,  float Match, float Cooldown,float StartingTime)
{
	WarmUpTime = WarmUp;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (BlockHUD && MatchState == MatchState::WaitingToStart)
	{
		BlockHUD->AddAnnouncement();
	}
}

void ABlockPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(InPawn);
	if (BlockCharacter)
	{
		SetHUDHealth(BlockCharacter->GetHealth(), BlockCharacter->GetMaxHealth());
		SetHUDShield(BlockCharacter->GetShield(), BlockCharacter->GetMaxShield());
	}
	HideDeathMessage();
}

void ABlockPlayerController::UpdateDeathMessageHUD(const FString KilledBy)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	if (BlockHUD &&
		BlockHUD->CharacterOverlay &&
		BlockHUD->CharacterOverlay->DeathMessage &&
		BlockHUD->CharacterOverlay->KilledBy)
	{
		BlockHUD->CharacterOverlay->KilledBy->SetText(FText::FromString(KilledBy));
		BlockHUD->CharacterOverlay->KilledBy->SetVisibility(ESlateVisibility::Visible);
		BlockHUD->CharacterOverlay->DeathMessage->SetVisibility(ESlateVisibility::Visible);
	}
}

void ABlockPlayerController::HideDeathMessage()
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	if (BlockHUD &&
		BlockHUD->CharacterOverlay &&
		BlockHUD->CharacterOverlay->DeathMessage &&
		BlockHUD->CharacterOverlay->KilledBy)
	{
		BlockHUD->CharacterOverlay->KilledBy->SetVisibility(ESlateVisibility::Collapsed);
		BlockHUD->CharacterOverlay->DeathMessage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABlockPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->HealthBar &&
				BlockHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlockHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlockHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlockPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->ShieldBar &&
				BlockHUD->CharacterOverlay->ShieldText;
	
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		BlockHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlockHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlockPlayerController::SetHUDScore(float Score)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlockHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlockPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->DefeatAmount;
	if (bHUDValid)
	{
		FString DefeatText = FString::Printf(TEXT("%d"), Defeats);
		BlockHUD->CharacterOverlay->DefeatAmount->SetText(FText::FromString(DefeatText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlockPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlockHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlockPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlockHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlockPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->MatchCountDown;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlockHUD->CharacterOverlay->MatchCountDown->SetText(FText());
			return;
		}
		
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60.f;
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlockHUD->CharacterOverlay->MatchCountDown->SetText(FText::FromString(CountdownText));
	}
}

void ABlockPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	
	bool bHUDValid = BlockHUD &&
		BlockHUD->Announcement &&
			BlockHUD->Announcement->WarmUpTime;
			
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlockHUD->Announcement->WarmUpTime->SetText(FText());
			return;
		}
		
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlockHUD->Announcement->WarmUpTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlockPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	bool bHUDValid = BlockHUD &&
		BlockHUD->CharacterOverlay &&
			BlockHUD->CharacterOverlay->GrenadeText;
	if (bHUDValid)
	{
		
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
		BlockHUD->CharacterOverlay->GrenadeText->SetText(FText::FromString(GrenadeText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void ABlockPlayerController::SetHUDTime()
{
	float TimeLeft= 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	/*if (HasAuthority())
	{
		BlockGameMode = BlockGameMode == nullptr ? Cast<ABlockGameMode>(UGameplayStatics::GetGameMode(this)) : BlockGameMode;
		if (BlockGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlockGameMode->GetCountDownTime() + LevelStartingTime);
		}
	}*/
	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	
	CountDownInt = SecondsLeft;
}

void ABlockPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlockHUD && BlockHUD->CharacterOverlay)
		{
			CharacterOverlay = BlockHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if(bInitializeShield) SetHUDShield(HUDShield,HUDMaxShield);
				if(bInitializeScore) SetHUDScore(HUDScore);
				if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				
				ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(GetPawn());
				if (BlockCharacter && BlockCharacter->GetCombat())
				{
					if(bInitializeGrenades) SetHUDGrenades(BlockCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void ABlockPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlockPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlockPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlockPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlockPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;
	
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStart(bTeamsMatch);
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlockPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStart();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlockPlayerController::HandleMatchHasStart(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamsScores = bTeamsMatch;
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	if (BlockHUD)
	{
		if (BlockHUD->CharacterOverlay == nullptr) BlockHUD->AddCharacterOverlay();
		if (BlockHUD->Announcement)
		{
			BlockHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void ABlockPlayerController::HandleCooldown()
{
	BlockHUD = BlockHUD == nullptr ? Cast<ABlockHUD>(GetHUD()) : BlockHUD;
	
	if (BlockHUD)
	{
		
		BlockHUD->CharacterOverlay->RemoveFromParent();
		
		bool bHUDValid = BlockHUD->Announcement && 
		BlockHUD->Announcement->AnnouncementText && 
			BlockHUD->Announcement->InfoText;
		
		if (bHUDValid)
		{
			BlockHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			BlockHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			
			ABlockGameState* BlockGameState = Cast<ABlockGameState>(UGameplayStatics::GetGameState(this));
			ABlockPlayerState* BlockPlayerState = GetPlayerState<ABlockPlayerState>();
			if (BlockGameState && BlockPlayerState)
			{
				TArray<ABlockPlayerState*> TopPlayers = BlockGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamsScores ? GetTeamsInfoText(BlockGameState) : GetInfoText(TopPlayers);
				
				BlockHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(GetPawn());
	if (BlockCharacter && BlockCharacter->GetCombat())
	{
		BlockCharacter->bDisableGamePlay = true;
		BlockCharacter->GetCombat()->FireButtonPressed(false);
		
	}
}

FString ABlockPlayerController::GetInfoText(const TArray<ABlockPlayerState*>& Players)
{
	ABlockPlayerState* BlockPlayerState = GetPlayerState<ABlockPlayerState>();
	if (BlockPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == BlockPlayerState )
	{
		InfoTextString = Announcement::YouAreWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayerTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

FString ABlockPlayerController::GetTeamsInfoText(class ABlockGameState* BlockGameState)
{
	if (BlockGameState == nullptr) return FString();
	FString InfoTextString;
	
	const int32 RedTeamScore = BlockGameState->RedTeamScore;
	const int32 BlueTeamScore = BlockGameState->BlueTeamScore;
	
	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}
	
	return InfoTextString;
}
