// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogWindow.h"

#include "Interfaces/DialogInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "UI/DialogFooterWidget.h"
#include "UI/DialogHeaderWidget.h"
#include "UI/DialogTextWidget.h"
#include "UI/DialogTopicWidget.h"

void UDialogWindow::InitDialogWindow(UDialogComponent* InputDialogComponent, AActor* ActorDialog)
{
	check(InputDialogComponent);

	DialogActor = ActorDialog;


	IDialogInterface* DialogActorInterface = Cast<IDialogInterface>(DialogActor);
	if (DialogActor && DialogActorInterface)
	{
		RelationValue = DialogActorInterface->GetRelation(GetOwningPlayerPawn());
		RelationString = DialogActorInterface->GetRelationString(RelationValue);
		DialogComponent = DialogActorInterface->GetDialogComponent();
	}
	else
	{
		DialogComponent = InputDialogComponent;
		PostInitRelation();
	}

	Footer->InitDialog(this);
	Header->InitDialog(this);
	TopicText->InitDialog(this);
	TopicList->InitDialog(this);


	Header->SetRelationValue(RelationValue);
	Header->SetRelationString(RelationString);

	TopicText->ClearList();
	TopicList->UpdateTopicData();

	if (RelationValue >= DialogComponent->GetGreetingLimit())
		TopicText->AddEmptyTopicData(DialogComponent->GetGoodGreeting());
	else
		TopicText->AddEmptyTopicData(DialogComponent->GetBadGreeting());


	if (IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer()))
	{
		BearerInterface->GetQuestBearerComponent()->KnownQuestDispatcher.AddDynamic(
			this, &UDialogWindow::DisplayJournalUpdate);
	}


}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayJournalUpdate()
{
	TopicText->AddEmptyTopicData("<Italic>Your quest journal has been updated.</>");
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayDialogTopic(int64 ID)
{
	TopicText->AddTopicText(ID);
	//also try to update possible quest advancement here?

	if (IQuestGiverInterface* GiverInterface = Cast<IQuestGiverInterface>(DialogActor))
	{
		const FDialogTopicStruct& Topic = DialogComponent->GetDialogTopic(ID);

		if (IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer()))
		{
			if (Topic.QuestRelation.QuestID != 0)
			{
				for (auto& StepData : Topic.QuestRelation.Steps)
				{
					if (BearerInterface->CanValidate(Topic.QuestRelation.QuestID, StepData))
						BearerInterface->TryProgressQuest(
							Topic.QuestRelation.QuestID, DialogActor);
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayDialogTopicFromString(const FString& ID)
{
	DisplayDialogTopic(DialogComponent->GetDialogTopicID(ID));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::CloseWindow()
{
	if (IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer()))
		BearerInterface->GetQuestBearerComponent()->KnownQuestDispatcher.RemoveAll(this);

	OnExit.Broadcast();
}
