#include "UI/DialogTextWidget.h"
#include "Interfaces/DialogDisplayInterface.h"

FString UDialogTextWidget::ProcessText(const FString& InputString) const
{
	FString ScriptedText = InputString;

	if (const IDialogDisplayInterface* Displayer = Cast<IDialogDisplayInterface>(GetOwningPlayer()))
		ScriptedText = Displayer->ProcessScriptedFunction(ScriptedText, GetWorld());

	AActor* DialogActor = IDialogWindowInterface::Execute_GetDialogActor(ParentDialogObject.Get());
	return DialogComponent->ParseTextHyperlink(ScriptedText, DialogActor, GetOwningPlayer());
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::ClearList()
{
	if (ListViewWidget)
		ListViewWidget->ClearListItems();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::AddTopicData(const FDialogTextData& DialogTopic)
{
	if (!ListViewWidget)
		return;

	UDialogTextChunkData* Data = NewObject<UDialogTextChunkData>(this);
	Data->Data   = DialogTopic;
	Data->Parent = ParentDialogObject;
	ListViewWidget->AddItem(Data);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::AddEmptyTopicData(const FString& DialogText)
{
	FDialogTextData TextData;
	TextData.Id       = 0;
	TextData.TopicText = ProcessText(DialogText);
	AddTopicData(TextData);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent    = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::AddTopicText(int64 TopicID)
{
	const FDialogTopicStruct* Topic = DialogComponent->GetDialogTopicSafe(TopicID);
	if (!Topic)
		return;

	FDialogTextData TextData;
	TextData.Id               = Topic->Id;
	TextData.TopicName        = Topic->Topic;
	TextData.TopicText        = ProcessText(Topic->TopicText.ToString());
	TextData.VoiceoverCue     = Topic->VoiceoverCue;
	TextData.VoiceoverDuration = Topic->VoiceoverDuration;
	AddTopicData(TextData);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextWidget::ReprocessTopicLinks()
{
	if (!ListViewWidget)
		return;

	for (auto& DialogTextChunk : ListViewWidget->GetDisplayedEntryWidgets())
	{
		if (UDialogTextChunkWidget* DialogChunkWidget = Cast<UDialogTextChunkWidget>(DialogTextChunk))
		{
			const FText& CurrentText = DialogChunkWidget->GetTextData();
			FString NewText = ProcessText(CurrentText.ToString());
			DialogChunkWidget->SetTextData(FText::FromString(NewText));
		}
	}
}
