// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/DialogHeaderWidget.h"

void UDialogHeaderWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
	check(DialogComponent);
	SetDialogName(DialogComponent->GetDialogName());
}
