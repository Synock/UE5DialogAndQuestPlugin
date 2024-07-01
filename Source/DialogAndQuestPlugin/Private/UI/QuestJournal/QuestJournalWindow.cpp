// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/QuestJournal/QuestJournalWindow.h"
#include "Interfaces/QuestBearerInterface.h"

void UQuestJournalWindow::UpdateKnownQuest()
{
	if(ListWidget)
	{
		ListWidget->UpdateQuestList();

		if (CurrentQuestID == 0 && QuestComponent->GetAllKnownQuest().Num() > 0)
		{
			CurrentQuestID = QuestComponent->GetAllKnownQuest()[0].QuestID;
		}

		if(QuestComponent->GetAllKnownQuest().Num() > 0)
			DisplayQuest(CurrentQuestID);

	}

}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::InitJournal()
{
	if (IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(GetOwningPlayer()))
	{
		QuestComponent = QuestBearer->GetQuestBearerComponent();
		QuestComponent->KnownQuestDispatcher.AddDynamic(this, &UQuestJournalWindow::UpdateKnownQuest);
	}

	ListWidget->InitDialog(this);
	DetailsWidget->InitDialog(this);
	UpdateKnownQuest();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::DisplayQuest(int64 ID)
{
	CurrentQuestID = ID;
	DetailsWidget->DisplayQuestData(ID);
}
