// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTopicWidget.h"


void UDialogTopicWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}

<<<<<<< HEAD
void UDialogTopicWidget::UpdateTopicData()
{
	ClearList();
	for (auto& Topic : DialogComponent->GetAllDialogTopic())
	{
		if (Topic.TopicCondition.VerifyCondition(ParentDialog->GetDialogActor()))
=======
//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicWidget::UpdateTopicData()
{
	ClearList();

	for (auto& Topic : DialogComponent->GetAllDialogTopic())
	{
		if (Topic.TopicCondition.VerifyCondition(ParentDialog->GetDialogActor(), GetOwningPlayer()))
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
		{
			FDialogTextData TextData;
			TextData.Id = Topic.Id;
			TextData.TopicName = Topic.Topic;
			TextData.TopicText = Topic.TopicText;
			AddTopicData(TextData);
		}
	}
}
