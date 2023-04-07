// Fill out your copyright notice in the Description page of Project Settings.


#include "MainWidget.h"

#include "BattleGameStateBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "NetworkProject/NetworkProjectCharacter.h"

void UMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Player = Cast<ANetworkProjectCharacter>(GetOwningPlayerPawn());

	Button_Exit->OnClicked.AddDynamic(this, &UMainWidget::ExitSession);
}

void UMainWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Text_Health->SetText(FText::AsNumber(Player->GetHealth()));
	Text_Ammo->SetText(FText::AsNumber(Player->GetAmmo()));

	FString playerList;
	if(GetWorld()->GetGameState() != nullptr)
	{
		ABattleGameStateBase* battleGameState = Cast<ABattleGameStateBase>(GetWorld()->GetGameState());

		for (const auto& playerState : battleGameState->GetPlayerListByScore())
		{
			FString playerName = playerState->GetPlayerName();
			int32 playerScore = playerState->GetScore();
			playerList.Append(FString::Printf(TEXT("%s : %d\n"), *playerName, playerScore));
		}

		Text_PlayerList->SetText(FText::FromString(playerList));
	}
}

void UMainWidget::ExitSession()
{
	if(Player != nullptr)
	{
		Player->EndSession();
	}
}
