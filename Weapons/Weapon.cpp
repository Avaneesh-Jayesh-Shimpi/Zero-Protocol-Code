// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Character/BlockCharacter.h"
#include "CombatComponents/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Weapons/Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
//Components
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerController/BlockPlayerController.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget  = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::FireWeapon(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}	
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			const FRotator RandRotator = UKismetMathLibrary::RandomRotator();
			const FRotator LerpedRot = UKismetMathLibrary::RLerp(SocketTransform.GetRotation().Rotator(), RandRotator, .35f, true);
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
				CasingClass,
				SocketTransform.GetLocation(),
				LerpedRot
				);
			}
		}
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlockOwnerCharacter = nullptr;
	BlockOwnerController = nullptr;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket == nullptr) return FVector();
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::RandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;
	
	/*
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(),EndLoc, 4.f, 12.f, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(), 
		TraceStart, 
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), 
		FColor::Cyan, 
		true
		);
	*/
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	
	if (HasAuthority() && WeaponMesh)
	{
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	if (ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(OtherActor))
	{
		BlockCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(OtherActor))
	{
		BlockCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnPingToHigh(bool bPingToHigh)
{
	bUseServerSideRewind = !bPingToHigh;
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}


void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetReplicateMovement(false);
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	EnableCustomDepth(false);
	
	BlockOwnerCharacter = BlockOwnerCharacter == nullptr ? Cast<ABlockCharacter>(GetOwner()) : BlockOwnerCharacter;
	if (BlockOwnerCharacter && bUseServerSideRewindDefault)
	{
		BlockOwnerController = BlockOwnerController == nullptr ? Cast<ABlockPlayerController>(BlockOwnerCharacter->Controller) : BlockOwnerController;
		if (BlockOwnerController && HasAuthority() && !BlockOwnerController->HighPingDelegate.IsBound() && !BlockOwnerCharacter->IsLocallyControlled())
		{
			BlockOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingToHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SetReplicateMovement(true);
		
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
	
	BlockOwnerCharacter = BlockOwnerCharacter == nullptr ? Cast<ABlockCharacter>(GetOwner()) : BlockOwnerCharacter;
	if (BlockOwnerCharacter && bUseServerSideRewindDefault)
	{
		BlockOwnerController = BlockOwnerController == nullptr ? Cast<ABlockPlayerController>(BlockOwnerCharacter->Controller) : BlockOwnerController;
		if (BlockOwnerController && HasAuthority() && BlockOwnerController->HighPingDelegate.IsBound() && !BlockOwnerCharacter->IsLocallyControlled())
		{
			BlockOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingToHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetReplicateMovement(false);
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}
	
	BlockOwnerCharacter = BlockOwnerCharacter == nullptr ? Cast<ABlockCharacter>(GetOwner()) : BlockOwnerCharacter;
	if (BlockOwnerCharacter && bUseServerSideRewindDefault)
	{
		BlockOwnerController = BlockOwnerController == nullptr ? Cast<ABlockPlayerController>(BlockOwnerCharacter->Controller) : BlockOwnerController;
		if (BlockOwnerController && HasAuthority() && BlockOwnerController->HighPingDelegate.IsBound() && !BlockOwnerCharacter->IsLocallyControlled())
		{
			BlockOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingToHigh);
		}
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	BlockOwnerCharacter = BlockOwnerCharacter == nullptr ? Cast<ABlockCharacter>(GetOwner()) : BlockOwnerCharacter;
	if (BlockOwnerCharacter && BlockOwnerCharacter->GetCombat() && IsFull())
	{
		BlockOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		BlockOwnerCharacter = nullptr;
		BlockOwnerController = nullptr;
	}
	else
	{
		BlockOwnerCharacter = BlockOwnerCharacter == nullptr ? Cast<ABlockCharacter>(Owner) : BlockOwnerCharacter;
		if (BlockOwnerCharacter && BlockOwnerCharacter->GetEquippedWeapon() && BlockOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	BlockOwnerCharacter = BlockOwnerCharacter == nullptr ? Cast<ABlockCharacter>(GetOwner()) : BlockOwnerCharacter;
	if (BlockOwnerCharacter)
	{
		BlockOwnerController = BlockOwnerController == nullptr ? Cast<ABlockPlayerController>(BlockOwnerCharacter->Controller) : BlockOwnerController;
		if (BlockOwnerController)
		{
			BlockOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

bool AWeapon::IsEmpty()
{
	return Ammo <=0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}
