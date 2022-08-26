// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/DialogComponent.h"
#include "DialogWindow.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeButtonEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGiveButtonEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrainButtonEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBankButtonEvent);

/**
 * 
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogWindow : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	UDialogComponent* DialogComponent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogFooterWidget* Footer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogHeaderWidget* Header = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogTopicWidget* TopicList = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogTextWidget* TopicText = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UButton* TradeButton = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UButton* GiveButton = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UButton* TrainButton = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UButton* BankButton = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UWidgetSwitcher* WidgetSwitcher = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogTradeWidget* TradeWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogGiveWidget* GiveWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogTrainWidget* TrainWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogBankWidget* BankWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	AActor* DialogActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float RelationValue = 0.5f;

	UPROPERTY(BlueprintReadWrite)
	FString RelationString;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PostInitRelation();

	UFUNCTION(BlueprintCallable)
	void DisplayGiveWidget();

	UFUNCTION(BlueprintCallable)
	void DisplayTradeWidget();

	UFUNCTION(BlueprintCallable)
	void DisplayMainDialogWidget();

	UFUNCTION(BlueprintCallable)
	void DisplayTrainDialogWidget();

	UFUNCTION(BlueprintCallable)
	void DisplayBankDialogWidget();

public:

	UFUNCTION(BlueprintCallable)
	void AddTradeWidget(UDialogTradeWidget* TradeWidget);

	UFUNCTION(BlueprintCallable)
	void AddGiveWidget(UDialogGiveWidget* GiveWidget);

	UFUNCTION(BlueprintCallable)
	void AddTrainWidget(UDialogTrainWidget* TrainWidget);

	UFUNCTION(BlueprintCallable)
	void AddBankWidget(UDialogBankWidget* BankWidget);

	UFUNCTION(BlueprintCallable)
	const UDialogComponent* GetDialogComponent() const { return DialogComponent; }

	UFUNCTION(BlueprintCallable)
	const AActor* GetDialogActor() const { return DialogActor; }

	UFUNCTION(BlueprintCallable)
	void InitDialogWindow(UDialogComponent* InputDialogComponent, AActor* ActorDialog);

	UFUNCTION(BlueprintCallable)
	void DisplayDialogTopic(int64 ID);

	UFUNCTION(BlueprintCallable)
	void DisplayJournalUpdate();

	UFUNCTION(BlueprintCallable)
	void DisplayDialogTopicFromString(const FString& ID);

	UFUNCTION(BlueprintCallable)
	void DisplayPlainString(const FString& PlainString);

	UFUNCTION(BlueprintCallable)
	virtual void CloseWindow();

	UFUNCTION(BlueprintCallable)
	virtual void RefreshDialogOptions();

	UPROPERTY(BlueprintAssignable)
	FOnExitEvent OnExit;

	UPROPERTY(BlueprintAssignable)
	FOnTradeButtonEvent OnTrade;

	UPROPERTY(BlueprintAssignable)
	FOnGiveButtonEvent OnGive;

	UPROPERTY(BlueprintAssignable)
	FOnTrainButtonEvent OnTrain;

	UPROPERTY(BlueprintAssignable)
	FOnBankButtonEvent OnBank;
};
