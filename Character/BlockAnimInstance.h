// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlockTypes/TurningInPlace.h"
#include "Animation/AnimInstance.h"
#include "BlockAnimInstance.generated.h"

UCLASS()
class ZEROPROTOCOL_API UBlockAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	
private:
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	class ABlockCharacter* BlockCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bAiming;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	float YawOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	float Lean;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	float AO_Pitch;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	FTransform LeftHandTransform;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	ETurningInPlace TurningInPlace;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	FRotator RightHandRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bLocallyControlled;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bRotateRootBone;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bElimmed;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bUseFABRIK;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bUseAimOffsets;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta=(AllowPrivateAccess= "true"))
	bool bTransformRightHand;
};
