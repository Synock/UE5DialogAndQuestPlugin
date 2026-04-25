#include "UI/DialogTopicWidget.h"

void UDialogTopicWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent    = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicWidget::ClearList()
{
	if (TopicListView)
		TopicListView->ClearListItems();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicWidget::AddTopicData(const FDialogTextData& DialogTopic)
{
	if (!TopicListView)
		return;

	UDialogTextChunkData* Data = NewObject<UDialogTextChunkData>(this);
	Data->Data   = DialogTopic;
	Data->Parent = ParentDialogObject;
	TopicListView->AddItem(Data);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicWidget::UpdateTopicData()
{
	ClearList();

	if (!DialogComponent || !ParentDialogObject)
		return;

	AActor* DialogActor = IDialogWindowInterface::Execute_GetDialogActor(ParentDialogObject.Get());

	for (const auto& Topic : DialogComponent->GetAllDialogTopic())
	{
		if (Topic.TopicCondition.VerifyCondition(DialogActor, GetOwningPlayer()))
		{
			FDialogTextData TextData;
			TextData.Id        = Topic.Id;
			TextData.TopicName = Topic.Topic;
			TextData.TopicText = Topic.TopicText.ToString();
			AddTopicData(TextData);
		}
	}
}
