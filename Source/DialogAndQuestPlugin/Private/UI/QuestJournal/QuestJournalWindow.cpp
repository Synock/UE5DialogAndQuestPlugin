// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/QuestJournal/QuestJournalWindow.h"
<<<<<<< HEAD

=======
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
#include "Interfaces/QuestBearerInterface.h"

void UQuestJournalWindow::UpdateKnownQuest()
{
<<<<<<< HEAD
	ListWidget->UpdateQuestList();
=======
	if(ListWidget)
	{
		ListWidget->UpdateQuestList();

		if (CurrentQuestID == 0 && QuestComponent->GetAllKnownQuest().Num() > 0)
		{
			CurrentQuestID = QuestComponent->GetAllKnownQuest()[0].QuestID;
		}
		DisplayQuest(CurrentQuestID);
	}

>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::InitJournal()
{
<<<<<<< HEAD
	if(IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(GetOwningPlayer()))
=======
	if (IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(GetOwningPlayer()))
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	{
		QuestComponent = QuestBearer->GetQuestBearerComponent();
		QuestComponent->KnownQuestDispatcher.AddDynamic(this, &UQuestJournalWindow::UpdateKnownQuest);
	}

	ListWidget->InitDialog(this);
	DetailsWidget->InitDialog(this);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::DisplayQuest(int64 ID)
{
<<<<<<< HEAD
=======
	CurrentQuestID = ID;
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	DetailsWidget->DisplayQuestData(ID);
}
