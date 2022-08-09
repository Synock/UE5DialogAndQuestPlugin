// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/QuestJournal/QuestJournalStepWidget.h"

void UQuestJournalStepWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (const UQuestEntryData* Data = Cast<UQuestEntryData>(ListItemObject))
	{
		LocalData = Data->Data;
		ParentJournal = Data->Parent;
		InitData(Data->Data);
	}
}

//----------------------------------------------------------------------------------------------------------------------

FString UQuestJournalStepWidget::GetStrike(const FString& OriginalString)
{
	return "<Strike>" + OriginalString + "</>";
}

//----------------------------------------------------------------------------------------------------------------------

FString UQuestJournalStepWidget::GetBold(const FString& OriginalString)
{
	return "<Bold>" + OriginalString + "</>";
}