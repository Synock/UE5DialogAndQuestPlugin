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
		RelationValue = DialogActorInterface->GetRelation();
		RelationString = DialogActorInterface->GetRelationString(RelationValue);
		DialogComponent = DialogActorInterface->GetDialogComponent();
	}
	else
	{
		DialogComponent = InputDialogComponent;
		PostInitRelation();
	}


	InitDialogUI();
	Footer->InitDialog(this);
	Header->InitDialog(this);
	TopicText->InitDialog(this);
	TopicList->InitDialog(this);

	Header->SetRelationValue(RelationValue);
	Header->SetRelationString(RelationString);

	if (RelationValue >= DialogComponent->GetGreetingLimit())
		TopicText->AddEmptyTopicData(DialogComponent->GetGoodGreeting());
	else
		TopicText->AddEmptyTopicData(DialogComponent->GetBadGreeting());


	IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer());

	if (BearerInterface)
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

	IQuestGiverInterface* GiverInterface = Cast<IQuestGiverInterface>(DialogActor);

	if (GiverInterface)
	{
		const FDialogTopicStruct& Topic = DialogComponent->GetDialogTopic(ID);

		IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer());

		if (BearerInterface)
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
	IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer());

	if (BearerInterface)
		BearerInterface->GetQuestBearerComponent()->KnownQuestDispatcher.RemoveAll(this);

	RemoveFromParent();
}
