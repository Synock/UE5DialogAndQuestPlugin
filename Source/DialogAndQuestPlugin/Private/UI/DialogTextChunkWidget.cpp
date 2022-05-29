// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTextChunkWidget.h"

void UDialogTextChunkWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
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
