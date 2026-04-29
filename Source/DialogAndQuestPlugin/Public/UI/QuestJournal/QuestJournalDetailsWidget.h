#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/QuestBearerComponent.h"
#include "Quest/QuestData.h"
#include "QuestJournalDetailsWidget.generated.h"

/**
 * Right-page details widget — displays the selected quest's title, description, and step list.
 *
 * ## BindWidget requirements (Blueprint must have these exact names):
 *   - "QuestTitleText"       (UTextBlock)  — quest title header
 *   - "QuestDescriptionText" (UTextBlock)  — quest description (Mentioned: rumor text; Active/Finished: full description)
 *   - "StepListView"         (UListView)   — entry class must be UQuestJournalStepWidget
 *
 * ## Behavior by state:
 *   - Mentioned  : shows MentionedDescription; StepListView hidden (no steps yet)
 *   - Accepted / Achieved : shows QuestDescription; all previous steps (struck) + current step
 *   - Completed / Botched : shows QuestDescription; all steps struck through; StepListView visible
 *
 * ## Blueprint extension:
 *   Override OnQuestDisplayed / OnDetailsCleared for animations or extra visual updates.
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalDetailsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UTextBlock> QuestTitleText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UTextBlock> QuestDescriptionText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UListView> StepListView = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TObjectPtr<class UQuestJournalWindow> ParentJournal = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TObjectPtr<const UQuestBearerComponent> QuestBearerComponent = nullptr;

	/**
	 * Attempts to resolve ParentJournal and QuestBearerComponent by locating the owning
	 * UQuestJournalWindow via the UObject outer chain and calling InitJournal on it.
	 * Safe to call multiple times; no-ops once both references are valid.
	 */
	void TryAutoInit();

	/// Fires after C++ has fully populated the detail widgets. Override for visual polish.
	UFUNCTION(BlueprintNativeEvent, Category = "Journal")
	void OnQuestDisplayed(const FQuestProgressData& QuestProgress);
	virtual void OnQuestDisplayed_Implementation(const FQuestProgressData& QuestProgress) {}

	/// Fires when the detail area is cleared (no quest selected). Override for empty-state visuals.
	UFUNCTION(BlueprintNativeEvent, Category = "Journal")
	void OnDetailsCleared();
	virtual void OnDetailsCleared_Implementation() {}

public:
	/// Connect this widget to its parent journal and data source. Call once from InitJournal().
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void InitDialog(class UQuestJournalWindow* InputParentDialog);

	/// Populate the right page with data for the given quest ID.
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void DisplayQuestData(int64 QuestID);

	/// Clear all text and the step list.
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void ClearDetails();
};
