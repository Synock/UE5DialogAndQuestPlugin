// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTopicButtonWidget.h"

void UDialogTopicButtonWidget::InitParent(UDialogWindow* Parent)
{
	ParentDialog = Parent;
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogTopicButtonWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	UDialogTextChunkData* Data = Cast<UDialogTextChunkData>(ListItemObject);

	if (Data)
	{
		LocalData = Data->Data;
		ParentDialog = Data->Parent;
		InitData(Data->Data);
	}
}
