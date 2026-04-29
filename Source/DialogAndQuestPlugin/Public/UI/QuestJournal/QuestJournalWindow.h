#pragma once

#include "CoreMinimal.h"
#include "QuestJournalDetailsWidget.h"
#include "QuestJournalListWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/QuestBearerComponent.h"
#include "Quest/QuestData.h"
#include "QuestJournalWindow.generated.h"

/**
 * Book-style quest journal window with two pages:
 *   Left page  — three category tab buttons (Active / Finished / Rumored) + filtered quest list
 *   Right page — quest title, description, and step list for the selected quest
 *
 * ## BindWidget requirements (Blueprint must have these exact names):
 *   - "ActiveTabButton"   (UButton)                  — switches to Active category
 *   - "FinishedTabButton" (UButton)                  — switches to Finished category
 *   - "RumoredTabButton"  (UButton)                  — switches to Rumored category
 *   - "ListWidget"        (UQuestJournalListWidget)   — left-page quest list
 *   - "DetailsWidget"     (UQuestJournalDetailsWidget)— right-page quest details
 *
 * ## Lifecycle:
 *   NativeConstruct() wires tab button OnClicked delegates.
 *   Call InitJournal() after the widget is displayed to connect it to the owning player's
 *   UQuestBearerComponent and load the initial state.
 *
 * ## Blueprint extension:
 *   Override OnCategoryChanged to update tab button visuals (pressed/unpressed state).
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalWindow : public UUserWidget
{
	GENERATED_BODY()

protected:
	// --- Bound sub-widgets -------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UButton> ActiveTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UButton> FinishedTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UButton> RumoredTabButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UQuestJournalListWidget> ListWidget = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UQuestJournalDetailsWidget> DetailsWidget = nullptr;

	// --- State ------------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TObjectPtr<UQuestBearerComponent> QuestComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	int64 CurrentQuestID = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	EQuestJournalCategory ActiveCategory = EQuestJournalCategory::Active;

	bool bInitialized = false;

	// --- Internal ---------------------------------------------------------------

	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleVisibilityChanged(ESlateVisibility InVisibility);

	UFUNCTION()
	void OnActiveTabClicked();

	UFUNCTION()
	void OnFinishedTabClicked();

	UFUNCTION()
	void OnRumoredTabClicked();

	UFUNCTION()
	void UpdateKnownQuest();

	/**
	 * Fires whenever the active category tab changes.
	 * Override in Blueprint to update tab button pressed/highlighted styles.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Journal")
	void OnCategoryChanged(EQuestJournalCategory NewCategory);
	virtual void OnCategoryChanged_Implementation(EQuestJournalCategory NewCategory) {}

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Journal")
	const UQuestBearerComponent* GetQuestComponent() const { return QuestComponent; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Journal")
	EQuestJournalCategory GetActiveCategory() const { return ActiveCategory; }

	/// Connect the journal to the owning player's quest data and perform the initial refresh.
	/// Safe to call every time the journal is opened: first call performs full initialization,
	/// subsequent calls only refresh the displayed list and details.
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void InitJournal();

	/// Display the details for a specific quest and record it as CurrentQuestID.
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void DisplayQuest(int64 ID);

	/// Switch the active category tab, rebuild the list, and auto-select the first quest.
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void SetActiveCategory(EQuestJournalCategory Category);
};
