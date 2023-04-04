// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class NETWORKPROJECT_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UEditableText* EditText_ID;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UEditableText* EditText_RoomName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class USlider* Slider_PlayerCount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* Text_PlayerCount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_CreateSession;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Login;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_GoCreate;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_GoFind;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_CreateBack;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_FindBack;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_FindRefresh;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, meta = (BindWidget))
	class UScrollBox* ScrollBox_RoomList;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class USessionSlotWidget> SessionSlot;

protected:
	virtual void NativeConstruct() override;

private:
	class UServerGameInstance* GameInstance;


	UFUNCTION()
	void ClickLogin();

	UFUNCTION()
	void CreateServer();

	UFUNCTION()
	void OnMoveSlider(float Value);

	UFUNCTION()
	void GoCreate();

	UFUNCTION()
	void GoFind();

	UFUNCTION()
	void GoBack();

	UFUNCTION()
	void RefreshList();

	UFUNCTION()
	void RefreshEnabled();

	UFUNCTION()
	void AddNewSlot(FSessionInfo SessionInfo);
};
