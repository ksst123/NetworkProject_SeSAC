// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


// 생성자
UServerGameInstance::UServerGameInstance()
{
	SessionID = "Test Session";
}

// 최초 실행되는 함수 (= BeginPlay)
void UServerGameInstance::Init()
{
	Super::Init();

	// 세션을 생성한다.
	// 세션: 서버에서 생성되어 스트림이 연결되어 있는지 확인(?)
	// 인터페이스: 완전 추상 함수, 접두어 I를 갖는다
	
	// 온라인 세션 기능이 구현되어 있는 IOnlineSubsystem 클래스를 가져온다.
	IOnlineSubsystem* OnlineSubsys = IOnlineSubsystem::Get();
	if (OnlineSubsys)
	{
		// Ptr이 들어간 클래스는 언리얼 C++ 의 스마트 포인터가 아닌 네이티브 C++을 스마트 포인터처럼 사용할 수 있는 변수(포인터처럼 생기지 않은 포인터) 
		SessionInterface = OnlineSubsys->GetSessionInterface();

		if (SessionInterface != nullptr)
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UServerGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UServerGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UServerGameInstance::OnJoinSessionComplete);
		}

		// 연결 플랫폼의 이름을 출력한다.
		FString platformName = OnlineSubsys->GetSubsystemName().ToString();
		UE_LOG(LogTemp, Warning, TEXT("Platform Name: %s"), *platformName);
	}
}

void UServerGameInstance::CreateMySession(FString RoomName, int32 PlayerCount)
{
	if (SessionInterface != nullptr)
	{
		// 세션 생성 정보를 만든다.
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bAllowInvites = false;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowJoinViaPresence = true;
		SessionSettings.bIsDedicated = false;
		// 온라인 서브 시스템의 이름이 "NULL"이면 LAN 매칭을 하고, "Steam" 이면 스팀 서버로 매칭한다.
		SessionSettings.bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
		SessionSettings.NumPublicConnections = PlayerCount;
		SessionSettings.Set(FName("KEY_RoomName"), RoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
		SessionSettings.bShouldAdvertise = true;

		// 세션을 생성한다.
		SessionInterface->CreateSession(0, SessionID, SessionSettings);

		UE_LOG(LogTemp, Warning, TEXT("Create Session Try!"));
	}
}

// 만들어져 있는 세션을 찾는 함수
void UServerGameInstance::FindMySession()
{
	// 찾으려는 세션 쿼리를 생성한다.
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	SessionSearch->MaxSearchResults = 30;
	// 특정 키로 검색 조건을 필터링 하고 싶을 때 추가하는 쿼리
	// SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, FText("RoomName"), EOnlineComparisonOp::GreaterThanEquals);

	// Presence로 생성된 세션을 필터링하고 싶을 때
	// Advertise: 공개방 여부
	// Presence: 존재하는 레벨만 검색할 지 여부
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// 앞에서 만든 쿼리를 이용해서 세션을 찾는다.
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

// 찾은 세션 리스트 중에서 특정 세션에 들어가고자 할 때 사용할 함수
void UServerGameInstance::JoinMySession(int32 SessionIndex)
{
	// 인덱스로 세션을 선택하고 그 세션으로 조인한다.
	FOnlineSessionSearchResult SelectedSession = SessionSearch->SearchResults[SessionIndex];

	SessionInterface->JoinSession(0, SessionID, SelectedSession);
}

// 세션이 서버에 만들어졌을 때 호출될 함수
void UServerGameInstance::OnCreateSessionComplete(FName SessionName, bool bIsSucceeded)
{
	FString result = bIsSucceeded ? TEXT("Create Session Succeed!") : TEXT("Create Session Failed..");
	UE_LOG(LogTemp, Warning, TEXT("%s : %s"), *result, *SessionName.ToString());

	// 세션 생성에 성공했다면, 전투 맵 쪽으로 세션에 접속한 모든 인원을 이동시킨다.
	if (bIsSucceeded)
	{
		GetWorld()->ServerTravel("/Game/Maps/MainMap?Listen");
	}
}

// 세션 검색이 끝났을 때 호출될 함수
void UServerGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		TArray<FOnlineSessionSearchResult> SearchResults = SessionSearch->SearchResults;
		UE_LOG(LogTemp, Warning, TEXT("Find Session Count: %d"), SearchResults.Num());
		
		// 검색 결과들을 모두 순회한다.
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

			// 슬롯 생성에 필요한 정보를 이벤트로 송출한다.
			SearchResultDelegate.Broadcast(SearchedSessionInfo);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Find Session Failed..."));
	}

	// FindSession의 새로고침 버튼을 다시 활성화한다.
	SearchFinishedDelegate.Broadcast();
}

// 다른 세션에 합류 처리가 끝났을 때 호출되는 이벤트 함수
void UServerGameInstance::OnJoinSessionComplete(FName SessionName, enum EOnJoinSessionCompleteResult::Type JoinResult)
{
	// 만일 Join에 성공했다면
	if(JoinResult == EOnJoinSessionCompleteResult::Success)
	{
		FString joinAddress;

		// 세션 이름으로 IP 주소를 획득한다.
		// 넣은 세션 이름에 해당하는 IP 주소를 알려주는 함수
		SessionInterface->GetResolvedConnectString(SessionName, joinAddress);

		UE_LOG(LogTemp, Warning, TEXT("Join Address: %s"), *joinAddress);

		// 해당 IP Address로 레벨 이동을 한다.
		if(APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
		{
			PlayerController->ClientTravel(joinAddress, ETravelType::TRAVEL_Absolute);
		}
	}
}
