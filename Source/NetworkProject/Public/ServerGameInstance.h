// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ServerGameInstance.generated.h"

/**
 * 
 */

USTRUCT()
struct FSessionInfo
{
	GENERATED_BODY()

	FString RoomName;
	int32 CurrentPlayers;
	int32 MaxPlayers;
	int32 Ping;
	int32 Index;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSearchResult, FSessionInfo, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSearchFinished);

UCLASS()
class NETWORKPROJECT_API UServerGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UServerGameInstance();

	virtual void Init() override;


	// 전역 변수
	IOnlineSessionPtr SessionInterface;
	FName SessionID;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FOnSearchResult SearchResultDelegate;
	FOnSearchFinished SearchFinishedDelegate;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class USessionSlotWidget> SessionSlot;


	void CreateMySession(FString RoomName, int32 PlayerCount);
	void FindMySession();
	void JoinMySession(int32 SessionIndex);


	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bIsSucceeded);
	UFUNCTION()
	void OnFindSessionComplete(bool bWasSuccessful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);
};
