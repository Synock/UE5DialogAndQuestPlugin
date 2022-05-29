// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogFooterWidget.h"

void UDialogFooterWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}
