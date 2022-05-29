// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/QuestJournal/QuestJournalDetailsWidget.h"

#include "UI/QuestJournal/QuestJournalWindow.h"

void UQuestJournalDetailsWidget::InitDialog(UQuestJournalWindow* InputParentDialog)
{
	ParentJournal = InputParentDialog;
	QuestBearerComponent = InputParentDialog->GetQuestComponent();
}

<<<<<<< HEAD
=======
//----------------------------------------------------------------------------------------------------------------------

>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
void UQuestJournalDetailsWidget::DisplayQuestData(int64 QuestID)
{
	ClearData();
	AddQuestData(QuestBearerComponent->GetKnownQuest(QuestID));
<<<<<<< HEAD
}
=======
}
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
