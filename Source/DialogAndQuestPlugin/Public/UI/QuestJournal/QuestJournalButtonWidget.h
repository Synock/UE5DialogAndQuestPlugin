#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Quest/QuestData.h"
#include "QuestJournalButtonWidget.generated.h"

/**
 * List entry widget representing a single quest title in the journal list page.
 *
 * ## BindWidget requirements (Blueprint must have these exact names):
 *   - "QuestTitleText"   (UTextBlock)  — displays the quest title
 *
 * ## Blueprint extension:
 *   Override OnButtonRefreshed to apply state-specific styling (color, font weight, etc.)
 *   after C++ has populated the title text.
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalButtonWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	/// Bound widget — Blueprint must contain a UTextBlock named "QuestTitleText".
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UTextBlock> QuestTitleText = nullptr;

	/// Bound widget — the clickable button that triggers quest selection.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<UButton> QuestButton = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TObjectPtr<class UQuestJournalWindow> ParentJournal = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	FQuestProgressData LocalData;

	/**
	 * Called after C++ populates the title text.
	 * Override in Blueprint to apply state-driven styling (color for Botched, bold for active, etc.).
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Journal")
	void OnButtonRefreshed(const FQuestProgressData& QuestData);
	virtual void OnButtonRefreshed_Implementation(const FQuestProgressData& QuestData) {}

	virtual void NativeConstruct() override;

public:
	/// Call from Blueprint button OnClicked to select this quest in the journal.
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void SelectQuest();

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
