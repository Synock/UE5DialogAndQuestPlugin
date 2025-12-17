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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepairButtonEvent);

/**
 * 
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogWindow : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Dialog state
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UDialogComponent> DialogComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<AActor> DialogActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	float RelationValue = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	FString RelationString;

	// Widget bindings - these must match widget names in the Blueprint
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogFooterWidget> Footer = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogHeaderWidget> Header = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogTopicWidget> TopicList = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogTextWidget> TopicText = nullptr;


	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog|Widgets")
	TObjectPtr<class UWidgetSwitcher> WidgetSwitcher = nullptr;

	// Optional widget bindings - dynamically added child widgets
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogTradeWidget> TradeWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogGiveWidget> GiveWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogTrainWidget> TrainWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogBankWidget> BankWidgetPointer = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Widgets")
	TObjectPtr<class UDialogRepairWidget> RepairWidgetPointer = nullptr;

	// UUserWidget interface
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PostInitRelation();

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
	void AddRepairWidget(UDialogRepairWidget* RepairWidget);

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

	UPROPERTY(BlueprintAssignable)
	FOnRepairButtonEvent OnRepair;

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

	UFUNCTION(BlueprintCallable)
	void DisplayRepairDialogWidget();
};
