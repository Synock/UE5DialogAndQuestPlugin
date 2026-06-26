#include "UI/DialogGiveWidget.h"
#include "Components/Button.h"

void UDialogGiveWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (GiveButton)   GiveButton->OnClicked.AddUniqueDynamic(this,   &UDialogGiveWidget::OnGiveButtonClicked);
	if (CancelButton) CancelButton->OnClicked.AddUniqueDynamic(this, &UDialogGiveWidget::OnCancelButtonClicked);
}

void UDialogGiveWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent    = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
}

void UDialogGiveWidget::OnGiveButtonClicked()   { OnGive.Broadcast(); }
void UDialogGiveWidget::OnCancelButtonClicked() { OnCancel.Broadcast(); }
