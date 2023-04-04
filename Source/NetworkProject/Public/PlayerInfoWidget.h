// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKPROJECT_API UPlayerInfoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Text_Name;
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* ProgressBar_HP;


	void SetPlayer(class ANetworkProjectCharacter* Player);

	UFUNCTION()
	void SetHealthBar(const int32& Value);

	UFUNCTION(Server, Unreliable)
	void ServerSetHealthBar(const int32& Value);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSetHealthBar(const int32& Value);

	UFUNCTION(Client, Unreliable)
	void ClientSetHealthBar(const int32& Value);

private:
	class ANetworkProjectCharacter* Player;
};