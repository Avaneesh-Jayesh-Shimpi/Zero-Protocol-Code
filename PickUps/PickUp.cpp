// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Weapons/WeaponType/WeaponTypes.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
//sound
#include "MetasoundSource.h"

APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));
	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickUpMesh");
	PickUpMesh->SetupAttachment(OverlapSphere);
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickUpMesh->SetRelativeScale3D(FVector(3.5f, 3.5f, 3.5f));
	PickUpMesh->SetRenderCustomDepth(true);
	PickUpMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	
	PickUpEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickUpEffectComponent"));
	PickUpEffectComponent->SetupAttachment(RootComponent);
}

void APickUp::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&APickUp::BindOverlapTimerFinished,
			BindOverlapTime
		);
	}
}

void APickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APickUp::BindOverlapTimerFinished()
{
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickUp::OnSphereOverlap);
}

void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickUpMesh)
	{
		PickUpMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
}

void APickUp::Destroyed()
{
	Super::Destroyed();
	
	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickUpSound, GetActorLocation());
	}
	if (PickUpEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				this,
				PickUpEffect,
				GetActorLocation(),
				GetActorRotation()
				);
	}
}

