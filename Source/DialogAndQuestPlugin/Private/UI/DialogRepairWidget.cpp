#include "UI/DialogRepairWidget.h"
#include "UI/DialogWindow.h"

//----------------------------------------------------------------------------------------------------------------------

void UDialogRepairWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogRepairWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
}
