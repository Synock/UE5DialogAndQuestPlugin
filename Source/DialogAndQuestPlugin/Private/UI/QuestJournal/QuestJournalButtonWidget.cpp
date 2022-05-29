// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/QuestJournal/QuestJournalButtonWidget.h"

void UQuestJournalButtonWidget::InitParent(UQuestJournalWindow* Parent)
{
	ParentJournal = Parent;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalButtonWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (const UQuestJournalTitleData* Data = Cast<UQuestJournalTitleData>(ListItemObject))
	{
		LocalData = Data->Data;
		ParentJournal = Data->Parent;
		InitData(Data->Data);
	}
}
