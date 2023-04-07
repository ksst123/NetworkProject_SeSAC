// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKPROJECT_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Anim Settings")
	float Angle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Anim Settings")
	float MoveSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Anim Settings")
	bool bIsJumping = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Anim Settings")
	float AimAnglePitch = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Anim Settings")
	bool bIsDead = false;

	UFUNCTION()
	void AnimNotify_FootstepSound(class USoundBase* Source, class USoundAttenuation* Attenuation);

private:
	class ANetworkProjectCharacter* Player;
};
