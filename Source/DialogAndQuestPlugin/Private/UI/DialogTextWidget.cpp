// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTextWidget.h"

void UDialogTextWidget::AddEmptyTopicData(const FString& DialogText)
{
	FDialogTextData TextData;
	TextData.Id = 0;
	TextData.TopicText = DialogText;
	AddTopicData(TextData);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::AddTopicText(int64 TopicID)
{
	const FDialogTopicStruct& Topic = DialogComponent->GetDialogTopic(TopicID);
	FDialogTextData TextData;
	TextData.Id = Topic.Id;
	TextData.TopicName = Topic.Topic;
	TextData.TopicText = DialogComponent->ParseTextHyperlink(Topic.TopicText, ParentDialog->GetDialogActor());
	AddTopicData(TextData);
}
