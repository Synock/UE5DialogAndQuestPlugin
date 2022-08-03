// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTextWidget.h"

#include "Interfaces/DialogDisplayInterface.h"

void UDialogTextWidget::AddEmptyTopicData(const FString& DialogText)
{
	FDialogTextData TextData;
	TextData.Id = 0;
	FString ScriptedText = DialogText;

	if(const IDialogDisplayInterface* Displayer = Cast<IDialogDisplayInterface>(GetOwningPlayer()))
	{
		ScriptedText = Displayer->ProcessScriptedFunction(ScriptedText,GetWorld());
	}

	TextData.TopicText = DialogComponent->ParseTextHyperlink(ScriptedText, ParentDialog->GetDialogActor(),GetOwningPlayer());;
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
	FString ScriptedText = Topic.TopicText;

	if(const IDialogDisplayInterface* Displayer = Cast<IDialogDisplayInterface>(GetOwningPlayer()))
	{
		ScriptedText = Displayer->ProcessScriptedFunction(ScriptedText,GetWorld());
	}

	TextData.TopicText = DialogComponent->ParseTextHyperlink(ScriptedText, ParentDialog->GetDialogActor(),GetOwningPlayer());
	AddTopicData(TextData);
}
