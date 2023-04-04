// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "../NetworkProjectCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


void UPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Player = Cast<ANetworkProjectCharacter>(GetOwningActor());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(Player == nullptr)
	{
		return;
	}

	MoveSpeed = Player->GetVelocity().Length();

#pragma region CalculateAngle
	//// FVector forwardVector = Player->GetActorForwardVector();
	//FVector forwardVector = FRotationMatrix(Player->GetActorRotation()).GetUnitAxis(EAxis::X);
	//FVector normalVelocity = Player->GetVelocity().GetSafeNormal2D();

	//// 회전 각도 계산
	//double cosTheta = FVector::DotProduct(forwardVector, normalVelocity);
	//double radian = FMath::Acos(cosTheta);
	//float degree = FMath::RadiansToDegrees(radian);

	//FVector rightVector = FRotationMatrix(Player->GetActorRotation()).GetUnitAxis(EAxis::Y);

	//// 회전 방향 계산
	//cosTheta = FVector::DotProduct(rightVector, normalVelocity);
	//Angle = cosTheta >= 0 ? degree : degree * -1;
#pragma endregion

	Angle = UKismetAnimationLibrary::CalculateDirection(Player->GetVelocity(), Player->GetActorRotation());

	bIsJumping = Player->GetCharacterMovement()->IsFalling();

	FRotator viewRotation = Player->GetBaseAimRotation();
	FRotator playerRotation = Player->GetActorRotation();
	FRotator deltaRotation = playerRotation - viewRotation;
	AimAnglePitch = FMath::Clamp(deltaRotation.GetNormalized().Pitch, -45.f, 45.f);

	bIsDead = Player->IsDead();
}