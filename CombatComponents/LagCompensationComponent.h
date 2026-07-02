// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/RingBuffer.h"
#include "LagCompensationComponent.generated.h"

class ABlockCharacter;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector Location;
	
	UPROPERTY()
	FRotator Rotation;
	
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct  FFramePackage
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Time = 0.f;
	
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
	
	UPROPERTY()
	ABlockCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool bHitConfirmed;
	
	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotGunServerSideRewindResult
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<ABlockCharacter*, uint32> HeadShots;
	
	UPROPERTY()
	TMap<ABlockCharacter*, uint32> BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZEROPROTOCOL_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend  class ABlockCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);
	
	/*
	 * HitScan Weapons
	 */
	
	FServerSideRewindResult ServerSideRewind(
		ABlockCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlockCharacter* HitCharacter, 
		const FVector_NetQuantize TraceStart, 
		const FVector_NetQuantize HitLocation,
		float HitTime);
	
	/*
	 * Projectile Weapon
	 */
	FServerSideRewindResult ProjectileServerSideRewind(
	ABlockCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize100& InitialVelocity,
	float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		ABlockCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);
	
	/*
	 * ShotGun Weapon
	 */
	
	FShotGunServerSideRewindResult ShotGunServerSideRewind(
		const TArray<ABlockCharacter*>& HitCharacters, 
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
	
	UFUNCTION(Server, Reliable)
	void ShotGunServerScoreRequest(
		const TArray<ABlockCharacter*>& HitCharacters, 
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);
	
protected:
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrame(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

	void CacheBoxPositions(ABlockCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ABlockCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ABlockCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ABlockCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(ABlockCharacter* HitCharacter, float HitTime); 	
	
	/*
	 * HitScan
	 */
	
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package, 
		ABlockCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation);
	
	/*
	 * Projectile Weapon
	 */
	
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package, 
		ABlockCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);
	
	/*
	 * ShotGun
	 */
	
	FShotGunServerSideRewindResult ShotGunConfirmHit(
		const TArray<FFramePackage>& FramePackages, 
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations);
	
	void CheckForBodyShots(
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations,
    	 FShotGunServerSideRewindResult& ShotGunResult, 
    	 UWorld* World);
	
	void CheckForHeadShots(
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations,
		FShotGunServerSideRewindResult& ShotGunResult, 
		UWorld* World);
	
private:
	UPROPERTY()
	ABlockCharacter* Character;
	
	UPROPERTY()
	class ABlockPlayerController* Controller;
	
	TDoubleLinkedList<FFramePackage> FrameHistory;
	
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:

		
};
	