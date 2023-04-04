// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AActor* ABattleGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	Super::ChoosePlayerStart_Implementation(Player);

	// ���� �ȿ� �ִ� APlayerStart ���͵��� �����´�.
	for (TActorIterator<APlayerStart> itr(GetWorld()); itr; ++itr)
	{
		APlayerStart* playerStart = *itr;
		if (playerStart->PlayerStartTag != FName("Spawned"))
		{
			// ��ȯ�� APlayerStart ������ �±�"Spawned"�� �߰��Ѵ�.
			playerStart->PlayerStartTag = FName("Spawned");
			return playerStart;
		}
	}

	return nullptr;
}
