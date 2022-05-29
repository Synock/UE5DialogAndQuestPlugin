// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/QuestJournal/QuestJournalListWidget.h"
#include "UI/QuestJournal/QuestJournalWindow.h"

void UQuestJournalListWidget::InitDialog(UQuestJournalWindow* InputParentDialog)
{
	ParentJournal = InputParentDialog;
	QuestBearerComponent = InputParentDialog->GetQuestComponent();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalListWidget::UpdateQuestList()
{
	ClearList();
	check(QuestBearerComponent);
	check(ParentJournal);
	for (auto& QuestData : QuestBearerComponent->GetAllKnownQuest())
	{
<<<<<<< HEAD
		if (!QuestData.Finished)
			AddQuestTitleData(QuestData);
=======
		AddQuestTitleData(QuestData);
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	}
}
