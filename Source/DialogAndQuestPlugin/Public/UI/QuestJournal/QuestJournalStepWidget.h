#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/RichTextBlock.h"
#include "Quest/QuestData.h"
#include "QuestJournalStepWidget.generated.h"

/**
 * List entry widget representing a single quest step on the journal details page.
 *
 * ## BindWidget requirements (Blueprint must have these exact names):
 *   - "StepTitleText"       (URichTextBlock) — completed → <Strike>, active → <Bold>
 *   - "StepDescriptionText" (URichTextBlock) — completed → <Strike>, active → plain
 *
 * ## Blueprint extension:
 *   Override OnStepRefreshed for additional visual polish after C++ populates both fields.
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalStepWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<URichTextBlock> StepTitleText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Journal|Widgets")
	TObjectPtr<URichTextBlock> StepDescriptionText = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	FQuestProgressStep LocalData;

	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	TObjectPtr<class UQuestJournalWindow> ParentJournal = nullptr;

	/**
	 * True when this entry is a visual separator between branch alternatives ("— OR —").
	 * Blueprint should check this flag to swap to a divider visual and hide normal
	 * title/description fields.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Journal")
	bool bIsBranchSeparator = false;

	/** Called after C++ populates both text fields. Override in Blueprint for visual polish. */
	UFUNCTION(BlueprintNativeEvent, Category = "Journal")
	void OnStepRefreshed(const FQuestProgressStep& StepData);
	virtual void OnStepRefreshed_Implementation(const FQuestProgressStep& StepData) {}

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Journal|Text")
	static FString GetStrike(const FString& OriginalString);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Journal|Text")
	static FString GetBold(const FString& OriginalString);
};
