#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/DialogComponent.h"
#include "Interfaces/DialogWindowInterface.h"
#include "DialogWindow.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTradeButtonEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGiveButtonEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrainButtonEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBankButtonEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepairButtonEvent);

/**
 * Default concrete implementation of IDialogWindowInterface.
 *
 * Contains sub-widgets (UDialogHeaderWidget, UDialogTopicWidget, etc.) bound via
 * BindWidget macros and drives them from its _Implementation overrides. Existing
 * Blueprints that use UDialogWindow require no changes.
 *
 * Custom game implementations (e.g. UFinalDialogWindow) should implement
 * IDialogWindowInterface directly rather than inheriting from this class.
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogWindow : public UUserWidget, public IDialogWindowInterface
{
	GENERATED_BODY()

protected:
	// Dialog state ---------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UDialogComponent> DialogComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<AActor> DialogActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	float RelationValue = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	FString RelationString;

	// Widget bindings ------------------------------------------------------------
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

	// Optional sub-widgets -------------------------------------------------------
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

	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PostInitRelation();

public:
	// IDialogWindowInterface — data accessors ------------------------------------
	virtual UDialogComponent* GetDialogComponent_Implementation() const override { return DialogComponent.Get(); }
	virtual AActor*            GetDialogActor_Implementation()    const override { return DialogActor.Get();    }

	// IDialogWindowInterface — lifecycle -----------------------------------------
	virtual void InitDialogWindow_Implementation(UDialogComponent* InputDialogComponent, AActor* ActorDialog) override;
	virtual void CloseWindow_Implementation() override;
	virtual void RefreshDialogOptions_Implementation() override;

	// IDialogWindowInterface — topic display -------------------------------------
	virtual void DisplayDialogTopic_Implementation(int64 ID) override;
	virtual void DisplayDialogTopicFromString_Implementation(const FString& ID) override;
	virtual void DisplayPlainString_Implementation(const FString& PlainString) override;

	// IDialogWindowInterface — tab navigation ------------------------------------
	virtual void DisplayMainDialogWidget_Implementation() override;
	virtual void DisplayTradeWidget_Implementation()      override;
	virtual void DisplayGiveWidget_Implementation()       override;
	virtual void DisplayTrainDialogWidget_Implementation() override;
	virtual void DisplayBankDialogWidget_Implementation() override;
	virtual void DisplayRepairDialogWidget_Implementation() override;

	// Sub-widget registration (game-side uses these to attach game-specific widgets)
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

	// Accessors for game-specific derived widgets ---------------------------------
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog")
	UDialogTradeWidget*  GetTradeWidget()  const { return TradeWidgetPointer;  }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog")
	UDialogRepairWidget* GetRepairWidget() const { return RepairWidgetPointer; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog")
	UDialogHeaderWidget* GetHeader()    const { return Header;    }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog")
	UDialogFooterWidget* GetFooter()    const { return Footer;    }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog")
	UDialogTopicWidget*  GetTopicList() const { return TopicList; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog")
	UDialogTextWidget*   GetTopicText() const { return TopicText; }

	// Broadcast when the dialog closes (subscribe in game wrappers if needed).
	UPROPERTY(BlueprintAssignable) FOnExitEvent        OnExit;
	UPROPERTY(BlueprintAssignable) FOnTradeButtonEvent  OnTrade;
	UPROPERTY(BlueprintAssignable) FOnGiveButtonEvent   OnGive;
	UPROPERTY(BlueprintAssignable) FOnTrainButtonEvent  OnTrain;
	UPROPERTY(BlueprintAssignable) FOnBankButtonEvent   OnBank;
	UPROPERTY(BlueprintAssignable) FOnRepairButtonEvent OnRepair;

	// Journal update helper (called by dialog systems after quest-state changes).
	UFUNCTION(BlueprintCallable)
	void DisplayJournalUpdate();
};
