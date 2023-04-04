// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AActor* ABattleGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	Super::ChoosePlayerStart_Implementation(Player);

	// 월드 안에 있는 APlayerStart 액터들을 가져온다.
	for (TActorIterator<APlayerStart> itr(GetWorld()); itr; ++itr)
	{
		APlayerStart* playerStart = *itr;
		if (playerStart->PlayerStartTag != FName("Spawned"))
		{
			// 반환될 APlayerStart 액터의 태그"Spawned"를 추가한다.
			playerStart->PlayerStartTag = FName("Spawned");
			return playerStart;
		}
	}

	return nullptr;
}
