// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "MainWidget.h"
#include "BattleGameMode.h"
#include "NetworkProject/NetworkProjectCharacter.h"

void ABattlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if(MainWidgetFactory != nullptr && IsLocalController())
	{
		UMainWidget* mainWidget = CreateWidget<UMainWidget>(this, MainWidgetFactory);

		if(mainWidget)
		{
			mainWidget->AddToViewport();
		}
	}
}

// ĳ���� ��Ȱ �Լ�
void ABattlePlayerController::Respawn(class ANetworkProjectCharacter* player)
{
	if(HasAuthority() && player != nullptr)
	{
		ABattleGameMode* battleGameMode = Cast<ABattleGameMode>(GetWorld()->GetAuthGameMode());
		if(battleGameMode != nullptr)
		{
			player->Destroy();
			// FTimerHandle...
			battleGameMode->RestartPlayer(this);
		}
	}
}
