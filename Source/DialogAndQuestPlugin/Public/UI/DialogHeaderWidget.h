#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/DialogWindowInterface.h"
#include "DialogHeaderWidget.generated.h"

/**
 * Dialog header widget containing character name, relation info, and action buttons.
 * References the parent dialog via IDialogWindowInterface — works with any
 * implementing class (UDialogWindow, UFinalDialogWindow, or custom).
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogHeaderWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UObject> ParentDialogObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<const UDialogComponent> DialogComponent = nullptr;

	// --- Text displays (BindWidgetOptional so missing blocks are silently skipped) ---

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Text")
	TObjectPtr<class UTextBlock> NameText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Text")
	TObjectPtr<class UTextBlock> RelationValueText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Text")
	TObjectPtr<class UTextBlock> RelationStringText = nullptr;

	// --- Action buttons (optional; not all NPCs expose every service) ---

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

	virtual void NativeConstruct() override;

	UFUNCTION() void OnTradeButtonClicked();
	UFUNCTION() void OnGiveButtonClicked();
	UFUNCTION() void OnTrainButtonClicked();
	UFUNCTION() void OnBankButtonClicked();
	UFUNCTION() void OnRepairButtonClicked();

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	virtual void InitDialog(UObject* InputParentDialog);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog")
	void SetDialogName(const FString& Name);
	virtual void SetDialogName_Implementation(const FString& Name);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog")
	void SetRelationValue(float RelationFloat);
	virtual void SetRelationValue_Implementation(float RelationFloat);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog")
	void SetRelationString(const FString& Name);
	virtual void SetRelationString_Implementation(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Dialog|Buttons")
	void ConfigureButtons(bool bCanTrade, bool bCanGive, bool bCanTrain, bool bCanBank, bool bCanRepair);

	/**
	 * Enables or disables the service buttons (Trade, Train, Bank, Repair).
	 * Call with bEnabled=false when the player's relation is below the NPC's GreetingLimit
	 * so buttons remain visible but are greyed-out and non-interactive.
	 * Give is intentionally unaffected — players can always attempt to gift items.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialog|Buttons")
	void SetServiceButtonsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog|Buttons")
	UButton* GetTradeButton()  const { return TradeButton;  }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog|Buttons")
	UButton* GetGiveButton()   const { return GiveButton;   }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog|Buttons")
	UButton* GetTrainButton()  const { return TrainButton;  }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog|Buttons")
	UButton* GetBankButton()   const { return BankButton;   }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dialog|Buttons")
	UButton* GetRepairButton() const { return RepairButton; }
};
