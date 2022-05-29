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

<<<<<<< HEAD
FString UQuestJournalStepWidget::GetStrike(const FString& OriginalString)
{
	return "<Strike>"+OriginalString+"</>";
=======
//----------------------------------------------------------------------------------------------------------------------

FString UQuestJournalStepWidget::GetStrike(const FString& OriginalString)
{
	return "<Strike>" + OriginalString + "</>";
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}
