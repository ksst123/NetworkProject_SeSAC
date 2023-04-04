// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "BattleSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKPROJECT_API ABattleSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	class ANetworkProjectCharacter* OriginalPlayer;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void ChangeToPlayer();
};
