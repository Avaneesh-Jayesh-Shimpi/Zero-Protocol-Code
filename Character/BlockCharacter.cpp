// Fill out your copyright notice in the Description page of Project Settings.

//includes
#include "BlockCharacter.h"
#include "Weapons/Weapon.h"
#include "ZeroProtocol.h"
#include "Kismet/GameplayStatics.h"
//Components
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatComponents/CombatComponent.h"
#include "CombatComponents/BuffComponent.h"
//#include  "Particles/ParticleSystemComponent.h"
#include "CombatComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
//Server
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
//AnimInstance
#include  "BlockAnimInstance.h"
//sound
#include "MetasoundSource.h"
//PlayerController
#include "PlayerController/BlockPlayerController.h"
//GameMode
#include "GameMode/BlockGameMode.h"
//Timer
#include "TimerManager.h"
//Player State
#include "PlayerState/BlockPlayerState.h"
#include "GameState/BlockGameState.h"
//Weapon Type
#include "Weapons/WeaponType/WeaponTypes.h"
//PlayerStart
#include "PlayerStart/TeamPlayerStart.h"


ABlockCharacter::ABlockCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ViewCamera->bUsePawnControlRotation = false;
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
	
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);
	
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);
	
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
	
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	/**
	 * Hit Boxes for Server-side rewind
	 */
	//Head
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);
	
	//pelvis
	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);
	
	//spine_02
	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);
	
	//spine_03
	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);
	
	//upperarm_l
	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);
	
	//upperarm_r
	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);
	
	//lowerarm_l
	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);
	
	//lowerarm_l
	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);
	
	//hand_l
	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);
	
	//hand_r
	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);
	
	//backpack
	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);
	
	//blanket
	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);
	
	//thigh_l
	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);
	
	//thigh_r
	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);
	
	//calf_l
	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);
	
	//calf_r
	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);
	
	//foot_l
	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);
	
	//foot_r
	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);
	
	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlockCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ABlockCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlockCharacter, Health);
	DOREPLIFETIME(ABlockCharacter, Shield); 
	DOREPLIFETIME(ABlockCharacter, bDisableGamePlay);
}

void ABlockCharacter::BeginPlay()
{
	Super::BeginPlay();
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
	if (BlockPlayerController)
	{
		BlockPlayerController->HideDeathMessage();
	}
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlockCharacter::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABlockCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RotateInPlace(DeltaTime);
	HideCharacterIfCameraClose();
	PollInit();
}

void ABlockCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlockCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

void ABlockCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || DefaultMaterial == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, DefaultMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
		
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
		
	case ETeam::ET_MAX:
		break;
	}
}

void ABlockCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGamePlay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlockCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ABlockCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlockCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABlockCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ABlockCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(CKeyAction, ETriggerEvent::Triggered, this, &ABlockCharacter::CrouchKeyPressed);
		EnhancedInputComponent->BindAction(AimKeyAction, ETriggerEvent::Triggered, this, &ABlockCharacter::AimKeyPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlockCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlockCharacter::FireButtonReleased);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABlockCharacter::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(GKeyAction, ETriggerEvent::Started, this, &ABlockCharacter::GrenadeButtonPressed);
		EnhancedInputComponent->BindAction(SwapKeyAction, ETriggerEvent::Completed, this, &ABlockCharacter::SwapWeapon);
	}

}

void ABlockCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, 
			GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlockPlayerController>(Controller);
		}
	}
}

void ABlockCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlockCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SMG:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("ShotGun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
			
		case EWeaponType::EWT_MAX:
			break;
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlockCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlockCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlockCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlockCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlockCharacter::OnPlayerStateInitialized()
{
	BlockPlayerState->AddToScore(0.f);
	BlockPlayerState->AddToDefeats(0);
	SetTeamColor(BlockPlayerState->GetTeam());
	SetSpawnPoint();
}

void ABlockCharacter::SetSpawnPoint()
{
	if (HasAuthority() && BlockPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == BlockPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
		}
	}
}

void ABlockCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                    class AController* InstigatorController, AActor* DamageCauser)
{
	BlockGameMode = BlockGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlockGameMode>() : BlockGameMode;
	if (bElimmed || BlockGameMode == nullptr) return;
	Damage = BlockGameMode->CalculateDamage(InstigatorController, Controller, Damage);
	
	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();
	
	if (Health == 0.f)
	{
		if (BlockGameMode)
		{
			BlockPlayerController = BlockPlayerController == nullptr ? Cast<ABlockPlayerController>(Controller) : BlockPlayerController;
			ABlockPlayerController* AttackerController = Cast<ABlockPlayerController>(InstigatorController);
			BlockGameMode->PlayerEliminated(this, BlockPlayerController, AttackerController);
		}
	}
}

void ABlockCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlockCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapon(); 
	MulticastElim(bPlayerLeftGame);
}

void ABlockCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (BlockPlayerController)
	{
		BlockPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bElimmed = true;
 	PlayElimMontage();
	
	//Start Dissolve Effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();
	
	//Disable Character Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGamePlay = true;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bool bHideSniperScope = IsLocallyControlled() && 
	Combat && 
	Combat->bAiming && 
	Combat->EquippedWeapon && 
	Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlockCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ABlockCharacter::DropOrDestroy(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlockCharacter::DropOrDestroyWeapon()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroy(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroy(Combat->SecondaryWeapon);
		}
	}
}


void ABlockCharacter::ElimTimerFinished()
{
	BlockGameMode = BlockGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlockGameMode>() : BlockGameMode;
	if (BlockGameMode && !bLeftGame)
	{
		BlockGameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABlockCharacter::ServerLeaveGame_Implementation()
{
	BlockGameMode = BlockGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlockGameMode>() : BlockGameMode;
	BlockPlayerState = BlockPlayerState == nullptr ? GetPlayerState<ABlockPlayerState>() : BlockPlayerState;
	if (BlockGameMode && BlockPlayerState)
	{
		BlockGameMode->PlayerLeftGame(BlockPlayerState);
	}
}

void ABlockCharacter::UpdateHUDHealth()
{
	BlockPlayerController = BlockPlayerController == nullptr ? Cast<ABlockPlayerController>(Controller) : BlockPlayerController;
	if (BlockPlayerController)
	{
		BlockPlayerController->SetHUDHealth(Health,MaxHealth);
	}
}

void ABlockCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlockCharacter::UpdateHUDShield()
{
	BlockPlayerController = BlockPlayerController == nullptr ? Cast<ABlockPlayerController>(Controller) : BlockPlayerController;
	if (BlockPlayerController)
	{
		BlockPlayerController->SetHUDShield(Shield,MaxShield);
	}
}

void ABlockCharacter::UpdateHUDAmmo()
{
	BlockPlayerController = BlockPlayerController == nullptr ? Cast<ABlockPlayerController>(Controller) : BlockPlayerController;
	if (BlockPlayerController && Combat && Combat->EquippedWeapon)
	{
		BlockPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		BlockPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ABlockCharacter::SpawnDefaultWeapon()
{
	BlockGameMode = BlockGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlockGameMode>() : BlockGameMode;
	UWorld* World = GetWorld();
	if (BlockGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void ABlockCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}


void ABlockCharacter::PollInit()
{
	if (BlockPlayerState == nullptr)
	{
		BlockPlayerState = GetPlayerState<ABlockPlayerState>();
		if (BlockPlayerState)
		{
			OnPlayerStateInitialized();
			
			ABlockGameState* BlockGameState = Cast<ABlockGameState>(UGameplayStatics::GetGameState(this));
			if (BlockGameState && BlockGameState->TopScoringPlayers.Contains(BlockPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlockCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGamePlay) return;
	const FVector2D MovementVector = Value.Get<FVector2D>(); 

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation = FRotator(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABlockCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ABlockCharacter::AimKeyPressed(const FInputActionValue& Value)
{
	if (bDisableGamePlay) return;
	if (!Combat) return;
	const bool bIsAiming = Value.Get<bool>();
	Combat->SetAiming(bIsAiming);
}

void ABlockCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlockCharacter::FireButtonPressed(const FInputActionValue& Value)
{
	if (bDisableGamePlay) return;
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	const bool bIsPressed = Value.Get<bool>();
	Combat->FireButtonPressed(bIsPressed);
}

void ABlockCharacter::FireButtonReleased()
{
	if (bDisableGamePlay) return;
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	Combat->FireButtonPressed(false);
}

void ABlockCharacter::ReloadButtonPressed()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		Combat->Reload();
	}
	
}

float ABlockCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlockCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	
	float Speed = CalculateSpeed();
	
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pitch();
}

void ABlockCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlockCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;	
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlockCharacter::SwapWeapon()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		if (Combat->CombatState == ECombatState::ECS_Unoccupied) ServerSwapWeapon();
		
		bool bSwapChecks =Combat->ShouldSwapWeapons() && 
			!HasAuthority() && 
				Combat->CombatState == ECombatState::ECS_Unoccupied &&
					OverlappingWeapon == nullptr;
		if (bSwapChecks)
		{
			PlaySwapMontage();
			Combat->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishSwapping = false;
		}
	}
	
}

void ABlockCharacter::ServerSwapWeapon_Implementation()
{
	if (Combat && Combat->ShouldSwapWeapons())
	{
		Combat->SwapWeapons();
	}
}

void ABlockCharacter::SpawnHitParticles(const FVector_NetQuantize& ImpactLocation, const FVector_NetQuantizeNormal& ImpactNormal)
{
	if (HitImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitImpactParticles, ImpactLocation, ImpactNormal.Rotation());
	}
    
	if (HitImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitImpactSound, ImpactLocation);
	}
}

void ABlockCharacter::Jump()
{
	if (bDisableGamePlay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlockCharacter::EKeyPressed()
{
	if (bDisableGamePlay) return;
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABlockCharacter::CrouchKeyPressed()
{
	if (bDisableGamePlay) return;
	if (GetCharacterMovement()->IsFalling())
	{
		return; 
	}
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlockCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
	}
}

void ABlockCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlockCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlockCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;
	if ((ViewCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlockCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlockCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlockCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlockCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlockCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlockCharacter::IsAiming()
{
	return (Combat && Combat->bAiming && Combat->EquippedWeapon);
}

AWeapon* ABlockCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlockCharacter::GetHitTarget() const
{
	
	if (Combat == nullptr)return FVector();
	return Combat->HitTarget;
}

ECombatState ABlockCharacter::GetCombatState() const
{
	if (Combat == nullptr)return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

void ABlockCharacter::Destroyed()
{
	Super::Destroyed();
	
	BlockGameMode = BlockGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlockGameMode>() : BlockGameMode;
	bool bMatchNotInProgress = BlockGameMode && BlockGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ABlockCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (HasAuthority() && Combat)
	{
		Combat->InitializeCarriedAmmo();
	}
}


bool ABlockCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}