#pragma once

#include "CoreMinimal.h"
#include "DialogWindow.h"
#include "Blueprint/UserWidget.h"
#include "DialogHeaderWidget.generated.h"

/**
 * Dialog header widget containing character name, relation info, and action buttons
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogHeaderWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// Dialog references
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UDialogWindow> ParentDialog = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<const UDialogComponent> DialogComponent = nullptr;

	// Button widgets - optional bind widgets (may or may not exist in Blueprint)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> TradeButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> GiveButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> TrainButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> BankButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> RepairButton = nullptr;

	// UUserWidget interface
	virtual void NativeConstruct() override;

	// Button click handlers
	UFUNCTION()
	void OnTradeButtonClicked();

	UFUNCTION()
	void OnGiveButtonClicked();

	UFUNCTION()
	void OnTrainButtonClicked();

	UFUNCTION()
	void OnBankButtonClicked();

	UFUNCTION()
	void OnRepairButtonClicked();

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void InitDialog(UDialogWindow* InputParentDialog);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Dialog")
	void SetDialogName(const FString& Name);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Dialog")
	void SetRelationValue(float RelationFloat);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Dialog")
	void SetRelationString(const FString& Name);

	// Button configuration methods
	UFUNCTION(BlueprintCallable, Category = "Dialog|Buttons")
	void ConfigureButtons(bool bCanTrade, bool bCanGive, bool bCanTrain, bool bCanBank, bool bCanRepair);

	// Button getters for DialogWindow compatibility
	UFUNCTION(BlueprintCallable, Category = "Dialog|Buttons")
	UButton* GetTradeButton() const { return TradeButton; }

	UFUNCTION(BlueprintCallable, Category = "Dialog|Buttons")
	UButton* GetGiveButton() const { return GiveButton; }

	UFUNCTION(BlueprintCallable, Category = "Dialog|Buttons")
	UButton* GetTrainButton() const { return TrainButton; }

	UFUNCTION(BlueprintCallable, Category = "Dialog|Buttons")
	UButton* GetBankButton() const { return BankButton; }

	UFUNCTION(BlueprintCallable, Category = "Dialog|Buttons")
	UButton* GetRepairButton() const { return RepairButton; }
};
