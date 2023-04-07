// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BattleGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKPROJECT_API ABattleGameStateBase : public AGameState
{
	GENERATED_BODY()

public:
	TArray<class APlayerState*> GetPlayerListByScore();
};
