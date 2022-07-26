// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTopicWidget.h"


void UDialogTopicWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicWidget::UpdateTopicData()
{
	ClearList();

	if(DialogComponent && ParentDialog)
	{
		for (auto& Topic : DialogComponent->GetAllDialogTopic())
		{
			if (Topic.TopicCondition.VerifyCondition(ParentDialog->GetDialogActor(), GetOwningPlayer()))
			{
				FDialogTextData TextData;
				TextData.Id = Topic.Id;
				TextData.TopicName = Topic.Topic;
				TextData.TopicText = Topic.TopicText;
				AddTopicData(TextData);
			}
		}
	}
}
