// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockAnimInstance.h"
#include "BlockCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/Weapon.h"
#include "BlockTypes/CombatState.h"

void UBlockAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	BlockCharacter = Cast<ABlockCharacter>(TryGetPawnOwner());
}

void UBlockAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	
	if (BlockCharacter == nullptr)
	{
		BlockCharacter = Cast<ABlockCharacter>(TryGetPawnOwner());
	}
	if (BlockCharacter == nullptr) return;

	FVector Velocity = BlockCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();

	bIsInAir = BlockCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlockCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = BlockCharacter->IsWeaponEquipped();
	EquippedWeapon = BlockCharacter->GetEquippedWeapon();
	bIsCrouched = BlockCharacter->bIsCrouched;
	bAiming = BlockCharacter->IsAiming();
	TurningInPlace = BlockCharacter->GetTurningInPlace();
	bRotateRootBone = BlockCharacter->ShouldRotateRootBone();
	bElimmed = BlockCharacter->IsElimmed();
	

	//Offset Yaw For Strafing
	FRotator AimRotation = BlockCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlockCharacter->GetVelocity());
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	//Lean 
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlockCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw/ DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlockCharacter->GetAO_Yaw();
	AO_Pitch = BlockCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlockCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		BlockCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		
		if (BlockCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(),RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlockCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	
	}
	
	bUseFABRIK = BlockCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bDisableFABRIK = BlockCharacter->IsLocallyControlled() && 
		BlockCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade &&
			BlockCharacter->bFinishSwapping;
	if (bDisableFABRIK)
	{
		bUseFABRIK = !BlockCharacter->IsLocallyReloading();
	}
	bUseAimOffsets = BlockCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlockCharacter->GetDisableGameplay();
	bTransformRightHand = BlockCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlockCharacter->GetDisableGameplay();
}
