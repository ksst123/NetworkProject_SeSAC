// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


// ������
UServerGameInstance::UServerGameInstance()
{
	SessionID = "Test Session";
}

// ���� ����Ǵ� �Լ� (= BeginPlay)
void UServerGameInstance::Init()
{
	Super::Init();

	// ������ �����Ѵ�.
	// ����: �������� �����Ǿ� ��Ʈ���� ����Ǿ� �ִ��� Ȯ��(?)
	// �������̽�: ���� �߻� �Լ�, ���ξ� I�� ���´�
	
	// �¶��� ���� ����� �����Ǿ� �ִ� IOnlineSubsystem Ŭ������ �����´�.
	IOnlineSubsystem* OnlineSubsys = IOnlineSubsystem::Get();
	if (OnlineSubsys)
	{
		// Ptr�� �� Ŭ������ �𸮾� C++ �� ����Ʈ �����Ͱ� �ƴ� ����Ƽ�� C++�� ����Ʈ ������ó�� ����� �� �ִ� ����(������ó�� ������ ���� ������) 
		SessionInterface = OnlineSubsys->GetSessionInterface();

		if (SessionInterface != nullptr)
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UServerGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UServerGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UServerGameInstance::OnJoinSessionComplete);
		}

		// ���� �÷����� �̸��� ����Ѵ�.
		FString platformName = OnlineSubsys->GetSubsystemName().ToString();
		UE_LOG(LogTemp, Warning, TEXT("Platform Name: %s"), *platformName);
	}
}

void UServerGameInstance::CreateMySession(FString RoomName, int32 PlayerCount)
{
	if (SessionInterface != nullptr)
	{
		// ���� ���� ������ �����.
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bAllowInvites = false;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.bIsDedicated = false;
		// �¶��� ���� �ý����� �̸��� "NULL"�̸� LAN ��Ī�� �ϰ�, "Steam" �̸� ���� ������ ��Ī�Ѵ�.
		SessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
		SessionSettings.NumPublicConnections = PlayerCount;
		SessionSettings.Set(FName("KEY_RoomName"), RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.bShouldAdvertise = true;

		// ������ �����Ѵ�.
		SessionInterface->CreateSession(0, SessionID, SessionSettings);

		UE_LOG(LogTemp, Warning, TEXT("Create Session Try!"));
	}
}

// ������� �ִ� ������ ã�� �Լ�
void UServerGameInstance::FindMySession()
{
	// ã������ ���� ������ �����Ѵ�.
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	SessionSearch->MaxSearchResults = 30;
	// Ư�� Ű�� �˻� ������ ���͸� �ϰ� ���� �� �߰��ϴ� ����
	// SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, FText("RoomName"), EOnlineComparisonOp::GreaterThanEquals);

	// Presence�� ������ ������ ���͸��ϰ� ���� ��
	// Advertise: ������ ����
	// Presence: �����ϴ� ������ �˻��� �� ����
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// �տ��� ���� ������ �̿��ؼ� ������ ã�´�.
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

// ã�� ���� ����Ʈ �߿��� Ư�� ���ǿ� ������ �� �� ����� �Լ�
void UServerGameInstance::JoinMySession(int32 SessionIndex)
{
	// �ε����� ������ �����ϰ� �� �������� �����Ѵ�.
	FOnlineSessionSearchResult SelectedSession = SessionSearch->SearchResults[SessionIndex];

	SessionInterface->JoinSession(0, SessionID, SelectedSession);
}

// ������ ������ ��������� �� ȣ��� �Լ�
void UServerGameInstance::OnCreateSessionComplete(FName SessionName, bool bIsSucceeded)
{
	FString result = bIsSucceeded ? TEXT("Create Session Succeed!") : TEXT("Create Session Failed..");
	UE_LOG(LogTemp, Warning, TEXT("%s : %s"), *result, *SessionName.ToString());

	// ���� ������ �����ߴٸ�, ���� �� ������ ���ǿ� ������ ��� �ο��� �̵���Ų��.
	if (bIsSucceeded)
	{
		GetWorld()->ServerTravel("/Game/Maps/MainMap?Listen");
	}
}

// ���� �˻��� ������ �� ȣ��� �Լ�
void UServerGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;
		UE_LOG(LogTemp, Warning, TEXT("Find Session Count: %d"), SearchResults.Num());
		
		// �˻� ������� ��� ��ȸ�Ѵ�.
		for (int32 i = 0; i < SearchResults.Num(); i++)
		{
			FSessionInfo SearchedSessionInfo;

			FString roomName;
			SearchResults[i].Session.SessionSettings.Get(FName("KEY_RoomName"), roomName);
			SearchedSessionInfo.RoomName = roomName;
			SearchedSessionInfo.MaxPlayers = SearchResults[i].Session.SessionSettings.NumPublicConnections;
			SearchedSessionInfo.CurrentPlayers = SearchedSessionInfo.MaxPlayers - SearchResults[i].Session.NumOpenPublicConnections;
			SearchedSessionInfo.Ping = SearchResults[i].PingInMs;
			SearchedSessionInfo.Index = i;

			// ���� ������ �ʿ��� ������ �̺�Ʈ�� �����Ѵ�.
			SearchResultDelegate.Broadcast(SearchedSessionInfo);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Find Session Failed..."));
	}

	// FindSession�� ���ΰ�ħ ��ư�� �ٽ� Ȱ��ȭ�Ѵ�.
	SearchFinishedDelegate.Broadcast();
}

// �ٸ� ���ǿ� �շ� ó���� ������ �� ȣ��Ǵ� �̺�Ʈ �Լ�
void UServerGameInstance::OnJoinSessionComplete(FName SessionName, enum EOnJoinSessionCompleteResult::Type JoinResult)
{
	// ���� Join�� �����ߴٸ�
	if(JoinResult == EOnJoinSessionCompleteResult::Success)
	{
		FString joinAddress;

		// ���� �̸����� IP �ּҸ� ȹ���Ѵ�.
		// ���� ���� �̸��� �ش��ϴ� IP �ּҸ� �˷��ִ� �Լ�
		SessionInterface->GetResolvedConnectString(SessionName, joinAddress);

		UE_LOG(LogTemp, Warning, TEXT("Join Address: %s"), *joinAddress);

		// �ش� IP Address�� ���� �̵��� �Ѵ�.
		if(APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
			PlayerController->ClientTravel(joinAddress, ETravelType::TRAVEL_Absolute);
		}
	}
}
