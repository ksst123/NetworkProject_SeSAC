// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleGameStateBase.h"

#include "GameFramework/PlayerState.h"

// 플레이어 스테이트 리스트를 점수에 따라서 내림차순으로 정렬하여 반환하는 함수
TArray<APlayerState*> ABattleGameStateBase::GetPlayerListByScore()
{
	TArray<APlayerState*> playerList = PlayerArray;

	playerList.Sort([](const APlayerState& a, const APlayerState& b) { return a.GetScore() > b.GetScore(); });

	return playerList;
}
