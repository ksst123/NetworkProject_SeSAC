// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BattleGameMode.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKPROJECT_API ABattleGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// AActor* ChoosePlayerStart(AController* Player);
	AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
