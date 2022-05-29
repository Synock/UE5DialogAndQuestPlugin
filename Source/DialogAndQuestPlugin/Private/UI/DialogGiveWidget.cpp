// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogGiveWidget.h"

void UDialogGiveWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}
