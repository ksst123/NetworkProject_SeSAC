// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerInfoWidget.h"

#include "Components/ProgressBar.h"
#include "NetworkProject/NetworkProjectCharacter.h"

void UPlayerInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Player = Cast<ANetworkProjectCharacter>(GetOwningPlayerPawn());

	if(Player != nullptr)
	{
		ServerSetHealthBar(Player->GetHealth());
	}
}

void UPlayerInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (Player == nullptr)
	{
		return;
	}
}

void UPlayerInfoWidget::SetPlayer(ANetworkProjectCharacter* player)
{
	Player = player;
}

void UPlayerInfoWidget::SetHealthBar(const int32& Value)
{
	if(Player)
	{
		float calcHP = (float)Value / (float)Player->MaxHP;
		ProgressBar_HP->SetPercent(calcHP);
	}
}

void UPlayerInfoWidget::ClientSetHealthBar_Implementation(const int32& Value)
{
	float calcHP = (float)Player->GetHealth() / (float)Player->MaxHP;
	ProgressBar_HP->SetPercent(calcHP);

	ServerSetHealthBar(0);
}

void UPlayerInfoWidget::MulticastSetHealthBar_Implementation(const int32& Value)
{
	float calcHP = (float)Player->GetHealth() / (float)Player->MaxHP;
	ProgressBar_HP->SetPercent(calcHP);
}

void UPlayerInfoWidget::ServerSetHealthBar_Implementation(const int32& Value)
{
	float calcHP = (float)Player->GetHealth() / (float)Player->MaxHP;
	ProgressBar_HP->SetPercent(calcHP);

	MulticastSetHealthBar(Value);
}
