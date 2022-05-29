// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/QuestJournal/QuestJournalDetailsWidget.h"

#include "UI/QuestJournal/QuestJournalWindow.h"

void UQuestJournalDetailsWidget::InitDialog(UQuestJournalWindow* InputParentDialog)
{
	ParentJournal = InputParentDialog;
	QuestBearerComponent = InputParentDialog->GetQuestComponent();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalDetailsWidget::DisplayQuestData(int64 QuestID)
{
	ClearData();
	AddQuestData(QuestBearerComponent->GetKnownQuest(QuestID));
}
