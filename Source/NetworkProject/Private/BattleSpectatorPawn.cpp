// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleSpectatorPawn.h"

#include "BattlePlayerController.h"
#include "NetworkProject/NetworkProjectCharacter.h"

void ABattleSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle respawnTimer;
	GetWorldTimerManager().SetTimer(respawnTimer, this, &ABattleSpectatorPawn::ChangeToPlayer, 3.f, false);
}

void ABattleSpectatorPawn::ChangeToPlayer()
{
	if(HasAuthority())
	{
		ABattlePlayerController* battlePlayerController = Cast<ABattlePlayerController>(GetController());
		if(battlePlayerController != nullptr && OriginalPlayer)
		{
			battlePlayerController->Possess(OriginalPlayer);
			battlePlayerController->Respawn(OriginalPlayer);
		}
	}
}
