
#include "UI/DialogTrainWidget.h"
#include "UI/DialogWindow.h"

void UDialogTrainWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}
