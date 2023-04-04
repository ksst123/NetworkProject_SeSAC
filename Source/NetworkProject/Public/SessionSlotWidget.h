// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKPROJECT_API USessionSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta=(BindWidget))
	class UButton* Button_RoomName;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Text_PlayerInfo;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Text_Ping;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_RoomName;
	UPROPERTY()
	int32 Index;


private:
	class UServerGameInstance* GameInstance;


	UFUNCTION()
	void JoinRoom();

};
