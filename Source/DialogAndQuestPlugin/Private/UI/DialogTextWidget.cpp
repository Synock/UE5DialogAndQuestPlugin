// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTextWidget.h"

#include "Interfaces/DialogDisplayInterface.h"

FString UDialogTextWidget::ProcessText(const FString& InputString) const
{
	FString ScriptedText = InputString;

	if(const IDialogDisplayInterface* Displayer = Cast<IDialogDisplayInterface>(GetOwningPlayer()))
	{
		ScriptedText = Displayer->ProcessScriptedFunction(ScriptedText,GetWorld());
	}

	return DialogComponent->ParseTextHyperlink(ScriptedText, ParentDialog->GetDialogActor(),GetOwningPlayer());;
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::AddEmptyTopicData(const FString& DialogText)
{
	FDialogTextData TextData;
	TextData.Id = 0;
	TextData.TopicText = ProcessText(DialogText);
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
	TextData.TopicText = ProcessText(Topic.TopicText);
	AddTopicData(TextData);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::ReprocessTopicLinks()
{
	for(auto& DialogTextChunk : ListViewWidget->GetDisplayedEntryWidgets())
	{
		if(UDialogTextChunkWidget* DialogChunkWidget = Cast<UDialogTextChunkWidget>(DialogTextChunk))
		{
			const FText& CurrentText = DialogChunkWidget->GetTextData();

			FString NewText = ProcessText(CurrentText.ToString());
			DialogChunkWidget->SetTextData(FText::FromString(NewText));
		}
	}
}
