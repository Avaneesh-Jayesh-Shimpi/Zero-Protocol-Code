#include "LagCompensationComponent.h"
#include "Character/BlockCharacter.h"
#include "Components/BoxComponent.h"
#include  "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Weapons/Weapon.h"
#include "ZeroProtocol.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlockCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}


FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlockCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotGunServerSideRewindResult ULagCompensationComponent::ShotGunServerSideRewind(
	const TArray<ABlockCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FrameToCheck;
	for (ABlockCharacter* HitCharacter : HitCharacters)
	{
		FrameToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotGunConfirmHit(FrameToCheck, TraceStart, HitLocations);
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlockCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Safely enable collision for the head first
	UBoxComponent** HeadBoxPtr = HitCharacter->HitCollisionBoxes.Find(FName("head"));
	if (HeadBoxPtr && *HeadBoxPtr)
	{
		(*HeadBoxPtr)->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		(*HeadBoxPtr)->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (ConfirmHitResult.bBlockingHit) // we hit the head, return early
		{
			/*if (ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
			}*/

			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else // didn't hit head, check the rest of the boxes
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if (ConfirmHitResult.bBlockingHit)
			{
				/*if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					}
				}*/

				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false };
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, ABlockCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Safely enable collision for the head first
	UBoxComponent** HeadBoxPtr = HitCharacter->HitCollisionBoxes.Find(FName("head"));
	if (HeadBoxPtr && *HeadBoxPtr)
	{
		(*HeadBoxPtr)->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		(*HeadBoxPtr)->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
    
	// Calculate exact flight time required to reach the target
	float Distance = FVector::Dist(TraceStart, HitCharacter->GetActorLocation());
	float Speed = InitialVelocity.Size();
	PathParams.MaxSimTime = (Speed > 0.f) ? (Distance / Speed) + 0.1f : 3.f; 

	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	/*PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;*/

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit) // we hit the head, return early
	{
		/*if (PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if (Box)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
			}
		}*/

		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{ true, true };
	}
	else // we didn't hit the head; check the rest of the boxes
	{
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}

		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			/*if (PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				}
			}*/

			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, false };
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{ false, false }; // Return false because trace missed entirely
}

FShotGunServerSideRewindResult ULagCompensationComponent::ShotGunConfirmHit(const TArray<FFramePackage>& FramePackages,
                                                                            const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr) return FShotGunServerSideRewindResult();
	}
	FShotGunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame : FramePackages)
	{
		// Safely enable collision for the head first
		UBoxComponent** HeadBoxPtr = Frame.Character->HitCollisionBoxes.Find(FName("head"));
		if (HeadBoxPtr && *HeadBoxPtr)
		{
			(*HeadBoxPtr)->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			(*HeadBoxPtr)->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
		}
	}

	UWorld* World = GetWorld();
	// check for head shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlockCharacter* BlasterCharacter = Cast<ABlockCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				/*if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
					}
				}*/

				if (ShotgunResult.HeadShots.Contains(BlasterCharacter))
				{
					ShotgunResult.HeadShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	// enable collision for all boxes, then disable for head box
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		
		// Safely disable head box
		UBoxComponent** HeadBoxPtr = Frame.Character->HitCollisionBoxes.Find(FName("head"));
		if (HeadBoxPtr && *HeadBoxPtr)
		{
			(*HeadBoxPtr)->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	// check for body shots
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlockCharacter* BlasterCharacter = Cast<ABlockCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				/*if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					}
				}*/

				if (ShotgunResult.BodyShots.Contains(BlasterCharacter))
				{
					ShotgunResult.BodyShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotgunResult;
}

FFramePackage ULagCompensationComponent::InterpBetweenFrame(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	
	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;
	
	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		
		const FBoxInformation* OlderBoxPtr = OlderFrame.HitBoxInfo.Find(BoxInfoName);
		const FBoxInformation* YoungerBoxPtr = YoungerFrame.HitBoxInfo.Find(BoxInfoName);
		if (OlderBoxPtr == nullptr || YoungerBoxPtr == nullptr) continue;
		const FBoxInformation& OlderBox = *OlderBoxPtr;
		const FBoxInformation& YoungerBox = *YoungerBoxPtr;
		
		FBoxInformation InterpBoxInfo;
		
		InterpBoxInfo.Location = FMath::VInterpTo(
			OlderBox.Location, 
			YoungerBox.Location, 
			1.f, 
			InterpFraction);
		
		InterpBoxInfo.Rotation = FMath::RInterpTo(
			OlderBox.Rotation, 
			YoungerBox.Rotation, 
			1.f, 
			InterpFraction);
		
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;
		
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
		
	}
	InterpFramePackage.Character = Character;
	return InterpFramePackage;
}

void ULagCompensationComponent::CacheBoxPositions(ABlockCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlockCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);
			
			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
			}
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlockCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);
			
			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlockCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}

}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlockCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();
	// Frame package that we check to verify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	// Frame history of the HitCharacter
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	
	if (OldestHistoryTime > HitTime)
	{
		// too far back - too laggy to do SSR
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime) // is Older still younger than HitTime?
	{
		// March back until: OlderTime < HitTime < YoungerTime
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime) // highly unlikely, but we found our frame to check
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	if (bShouldInterpolate)
	{
		// Interpolate between Younger and Older
		FrameToCheck = InterpBetweenFrame(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlockCharacter* HitCharacter,
                                                                  const FVector_NetQuantize TraceStart, const FVector_NetQuantize HitLocation, float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();
		
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlockCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	
	if (Character && HitCharacter && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();
		
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ShotGunServerScoreRequest_Implementation(const TArray<ABlockCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotGunServerSideRewindResult Confirm = ShotGunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	
	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || Character == nullptr) continue;
		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains((HitCharacter)))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains((HitCharacter)))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			HitCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Red);
	}

}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlockCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::CheckForBodyShots(const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, FShotGunServerSideRewindResult& ShotGunResult, UWorld* World)
{
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation-TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox);
			ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(ConfirmHitResult.GetActor());
			if (BlockCharacter)
			{
				/* if (ConfirmHitResult.Component.IsValid())
					{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				 	if (Box)
				 	{
				 		DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					}
				 }*/
				
				if (ShotGunResult.BodyShots.Contains(BlockCharacter))
				{
					ShotGunResult.BodyShots[BlockCharacter]++;
				}
				else
				{
					ShotGunResult.BodyShots.Emplace(BlockCharacter, 1);
				}
			}
		}
	}
}

void ULagCompensationComponent::CheckForHeadShots(const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, FShotGunServerSideRewindResult& ShotGunResult, UWorld* World)
{
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation-TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox);
			ABlockCharacter* BlockCharacter = Cast<ABlockCharacter>(ConfirmHitResult.GetActor());
			if (BlockCharacter)
			{
				/*if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
					}
				}*/
				
				if (ShotGunResult.HeadShots.Contains(BlockCharacter))
				{
					ShotGunResult.HeadShots[BlockCharacter]++;
				}
				else
				{
					ShotGunResult.HeadShots.Emplace(BlockCharacter, 1);
				}
			}
		}
	}
}