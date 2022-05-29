// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTopicButtonWidget.h"

void UDialogTopicButtonWidget::InitParent(UDialogWindow* Parent)
{
	ParentDialog = Parent;
}

<<<<<<< HEAD
=======
//----------------------------------------------------------------------------------------------------------------------

>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
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
