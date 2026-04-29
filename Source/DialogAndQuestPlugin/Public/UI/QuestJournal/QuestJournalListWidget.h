#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/QuestBearerComponent.h"
#include "Quest/QuestData.h"
#include "QuestJournalListWidget.generated.h"

/**
 * Left-page list widget — shows quest titles filtered by a single EQuestJournalCategory.
 *
 * ## BindWidget requirements (Blueprint must have these exact names):
 *   - "QuestListView"   (UListView) — entry class must be UQuestJournalButtonWidget
 *
 * ## Usage:
 *   1. Call InitDialog() once from UQuestJournalWindow::InitJournal().
 *   2. Call SetFilterCategory() to switch between Active/Finished/Rumored tabs.
 *   3. UpdateQuestList() is called automatically on category change and quest data change.
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalListWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/// Bound widget — Blueprint must contain a UTextBlock named "CategoryTitleText".
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UTextBlock> CategoryTitleText = nullptr;

	/// Bound widget — Blueprint must contain a UListView named "QuestListView".
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UListView> QuestListView = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TObjectPtr<class UQuestJournalWindow> ParentJournal = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TObjectPtr<const UQuestBearerComponent> QuestBearerComponent = nullptr;

	/// Category this list is currently displaying. Change via SetFilterCategory().
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Journal")
	EQuestJournalCategory FilterCategory = EQuestJournalCategory::Active;

	/**
	 * Attempts to resolve ParentJournal and QuestBearerComponent if they are null.
	 * ParentJournal is located by walking the UObject outer chain.
	 * QuestBearerComponent is fetched from the owning player via IQuestBearerInterface.
	 * Safe to call multiple times; does nothing once both references are valid.
	 */
	void TryAutoInit();

	void OnListItemClicked(UObject* Item);

public:
	/// Connect this list to its parent journal and data source. Call once from InitJournal().
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void InitDialog(class UQuestJournalWindow* InputParentDialog);

	/// Rebuild the list for the current FilterCategory from live quest data.
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void UpdateQuestList();

	/// Change the displayed category and immediately refresh the list.
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void SetFilterCategory(EQuestJournalCategory Category);

	/// Returns the QuestID of the first entry in the current filtered list, or 0 if empty.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Journal")
	int64 GetFirstQuestID() const;
};
