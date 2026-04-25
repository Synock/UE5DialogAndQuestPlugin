#include "UI/DialogRepairWidget.h"
#include "Components/Button.h"

//----------------------------------------------------------------------------------------------------------------------

void UDialogRepairWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UDialogRepairWidget::OnCloseButtonClicked);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogRepairWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent    = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogRepairWidget::OnCloseButtonClicked() { OnClose.Broadcast(); }
