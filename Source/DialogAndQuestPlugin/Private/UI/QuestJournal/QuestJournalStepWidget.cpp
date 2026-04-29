#include "UI/QuestJournal/QuestJournalStepWidget.h"
#include "Quest/QuestData.h"

void UQuestJournalStepWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const UQuestEntryData* Data = Cast<UQuestEntryData>(ListItemObject);
	if (!Data)
		return;

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