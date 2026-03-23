
#include "UI/DialogBankWidget.h"

#include "UI/DialogWindow.h"

void UDialogBankWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}
