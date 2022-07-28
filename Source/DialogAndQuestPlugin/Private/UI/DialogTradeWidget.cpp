// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogTradeWidget.h"

#include "UI/DialogWindow.h"

void UDialogTradeWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}
