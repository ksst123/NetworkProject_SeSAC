// Fill out your copyright notice in the Description page of Project Settings.


#include "MainWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "NetworkProject/NetworkProjectCharacter.h"

void UMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Player = Cast<ANetworkProjectCharacter>(GetOwningPlayerPawn());
}

void UMainWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Text_Health->SetText(FText::AsNumber(Player->GetHealth()));
	Text_Ammo->SetText(FText::AsNumber(Player->GetAmmo()));

	FString playerList;
	if(GetWorld()->GetGameState() != nullptr)
	{
		for (auto& playerState : GetWorld()->GetGameState()->PlayerArray)
		{
			FString playerName = playerState->GetPlayerName();
			playerList.Append(FString::Printf(TEXT("%s\n"), *playerName));
		}

		Text_PlayerList->SetText(FText::FromString(playerList));
	}
}
