#include "UI/DialogTopicButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UDialogTopicButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// TopicButton is bound via BindWidget; bind click once at construction.
	// ParentDialogObject and LocalData are populated later by NativeOnListItemObjectSet
	// before the user can interact, so reading them at click-time is safe.
	if (TopicButton)
		TopicButton->OnClicked.AddDynamic(this, &UDialogTopicButtonWidget::OnTopicButtonClicked);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicButtonWidget::InitParent(UObject* Parent)
{
	ParentDialogObject = Parent;
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicButtonWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	if (UDialogTextChunkData* Data = Cast<UDialogTextChunkData>(ListItemObject))
	{
		LocalData          = Data->Data;
		ParentDialogObject = Data->Parent;
		InitData(Data->Data);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicButtonWidget::InitData_Implementation(const FDialogTextData& ItemData)
{
	LocalData = ItemData;
	if (TopicLabel)
		TopicLabel->SetText(FText::FromString(ItemData.TopicName));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicButtonWidget::OnTopicButtonClicked()
{
	if (ParentDialogObject)
		IDialogWindowInterface::Execute_DisplayDialogTopic(ParentDialogObject.Get(), LocalData.Id);
}
