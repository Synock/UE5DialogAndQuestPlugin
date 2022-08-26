// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogBankWidget.h"

#include "UI/DialogWindow.h"

void UDialogBankWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}
