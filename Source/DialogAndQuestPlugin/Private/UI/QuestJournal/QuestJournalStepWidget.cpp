#include "UI/QuestJournal/QuestJournalStepWidget.h"
#include "Quest/QuestData.h"

void UQuestJournalStepWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const UQuestEntryData* Data = Cast<UQuestEntryData>(ListItemObject);
	if (!Data)
		return;

	// Branch-separator rows carry no step data — let the Blueprint handle the visual.
	// bIsBranchSeparator is exposed BlueprintReadOnly so the BP widget can switch its
	// visual state (e.g. show a centered "— OR —" label, hide normal title/desc fields).
	if (Data->bIsBranchSeparator)
	{
		bIsBranchSeparator = true;
		LocalData     = FQuestProgressStep{};
		ParentJournal = Data->Parent;
		// Explicitly clear any stale or designer-default (lorem ipsum) text so the
		// Blueprint is free to show a pure divider row without interference.
		if (StepTitleText)
			StepTitleText->SetText(FText::GetEmpty());
		if (StepDescriptionText)
			StepDescriptionText->SetText(FText::GetEmpty());
		OnStepRefreshed(LocalData);
		return;
	}

	bIsBranchSeparator = false;

	LocalData     = Data->Data;
	ParentJournal = Data->Parent;

	if (StepTitleText)
	{
		const FString TitleStr = LocalData.StepTitle.ToString();
		// Completed → strikethrough; active → bold (matches original Blueprint behavior)
		const FString RichTitle = LocalData.Completed ? GetStrike(TitleStr) : GetBold(TitleStr);
		StepTitleText->SetText(FText::FromString(RichTitle));
	}

	if (StepDescriptionText)
	{
		const FString DescStr = LocalData.StepDescription.ToString();
		// Mirror title: strike completed descriptions too
		const FString RichDesc = LocalData.Completed ? GetStrike(DescStr) : DescStr;
		StepDescriptionText->SetText(FText::FromString(RichDesc));
	}

	OnStepRefreshed(LocalData);
}

//----------------------------------------------------------------------------------------------------------------------

FString UQuestJournalStepWidget::GetStrike(const FString& OriginalString)
{
	return TEXT("<Strike>") + OriginalString + TEXT("</>");
}

//----------------------------------------------------------------------------------------------------------------------

FString UQuestJournalStepWidget::GetBold(const FString& OriginalString)
{
	return TEXT("<Bold>") + OriginalString + TEXT("</>");
}