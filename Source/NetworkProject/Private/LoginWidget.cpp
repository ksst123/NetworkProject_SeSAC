// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "ServerGameInstance.h"
#include "SessionSlotWidget.h"
#include "Blueprint/UserWidget.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 혹시 모를 입력 필드의 값을 빈 칸으로 초기화한다.
	EditText_ID->SetText(FText::FromString(""));
	EditText_RoomName->SetText(FText::FromString(""));
	Slider_PlayerCount->SetValue(2.0f);
	Text_PlayerCount->SetText(FText::FromString("2"));

	// 버튼 클릭 이벤트 함수 바인딩하기
	Button_Login->OnClicked.AddDynamic(this, &ULoginWidget::ClickLogin);
	Button_CreateSession->OnClicked.AddDynamic(this, &ULoginWidget::CreateServer);
	Slider_PlayerCount->OnValueChanged.AddDynamic(this, &ULoginWidget::OnMoveSlider);
	Button_GoCreate->OnClicked.AddDynamic(this, &ULoginWidget::GoCreate);
	Button_GoFind->OnClicked.AddDynamic(this, &ULoginWidget::GoFind);
	Button_CreateBack->OnClicked.AddDynamic(this, &ULoginWidget::GoBack);
	Button_FindBack->OnClicked.AddDynamic(this, &ULoginWidget::GoBack);
	Button_FindRefresh->OnClicked.AddDynamic(this, &ULoginWidget::RefreshList);

	GameInstance = Cast<UServerGameInstance>(GetGameInstance());

	if (GameInstance != nullptr)
	{
		GameInstance->SearchResultDelegate.AddDynamic(this, &ULoginWidget::AddNewSlot);
		GameInstance->SearchFinishedDelegate.AddDynamic(this, &ULoginWidget::RefreshEnabled);
	}
}

void ULoginWidget::ClickLogin()
{
	// 만일 ID가 빈칸이 아니라면 0번 -> 1번 캔버스로 변경한다.
	if (!EditText_ID->GetText().IsEmpty())
	{
		WidgetSwitcher->SetActiveWidgetIndex(1);
		GameInstance->SessionID = FName(*EditText_ID->GetText().ToString());
	}
}

void ULoginWidget::CreateServer()
{
	int32 playerCount = FMath::RoundHalfFromZero(Slider_PlayerCount->GetValue());

	GameInstance->CreateMySession(EditText_RoomName->GetText().ToString(), playerCount);
}

// 슬라이더 값이 변경될 때 호출되는 함수
void ULoginWidget::OnMoveSlider(float Value)
{
	FString NumToString = FString::Printf(TEXT("%d"), FMath::RoundToInt32(Value));
	Text_PlayerCount->SetText(FText::FromString(*NumToString));
}

void ULoginWidget::GoCreate()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);
}

void ULoginWidget::GoFind()
{
	WidgetSwitcher->SetActiveWidgetIndex(3);
	RefreshList();
}

void ULoginWidget::GoBack()
{
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void ULoginWidget::RefreshList()
{
	ScrollBox_RoomList->ClearChildren();
	GameInstance->FindMySession();
	Button_FindRefresh->SetIsEnabled(false);
}

void ULoginWidget::RefreshEnabled()
{
	Button_FindRefresh->SetIsEnabled(true);
}

// 게임 인스턴스로부터 검색 완료 이벤트를 받았을 때 실행될 함수
void ULoginWidget::AddNewSlot(FSessionInfo SessionInfo)
{
	USessionSlotWidget* SlotWidget = CreateWidget<USessionSlotWidget>(this, SessionSlot);

	if (SlotWidget != nullptr)
	{
		SlotWidget->Text_RoomName->SetText(FText::FromString(SessionInfo.RoomName));
		SlotWidget->Text_PlayerInfo->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), SessionInfo.CurrentPlayers, SessionInfo.MaxPlayers)));
		SlotWidget->Text_Ping->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), SessionInfo.Ping)));
		SlotWidget->Index = SessionInfo.Index;
		ScrollBox_RoomList->AddChild(SlotWidget);
	}
}
