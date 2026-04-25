#include "UI/DialogTextChunkWidget.h"

void UDialogTextChunkWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
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

void UDialogTextChunkWidget::InitData_Implementation(const FDialogTextData& ItemData)
{
	ItemID    = ItemData.Id;
	LocalData = ItemData;

	if (TextBlock)
		TextBlock->SetText(FText::FromString(ItemData.TopicText));

	if (TitleBlock)
		TitleBlock->SetText(FText::FromString(ItemData.TopicName));
}

//----------------------------------------------------------------------------------------------------------------------

FText UDialogTextChunkWidget::GetTextData() const
{
	return TextBlock ? TextBlock->GetText() : FText::GetEmpty();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTextChunkWidget::SetTextData(const FText& NewTextData)
{
	if (TextBlock)
		TextBlock->SetText(NewTextData);
}
