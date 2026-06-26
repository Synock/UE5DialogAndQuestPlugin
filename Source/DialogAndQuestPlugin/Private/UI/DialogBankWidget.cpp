#include "UI/DialogBankWidget.h"
#include "Components/Button.h"

void UDialogBankWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
		CloseButton->OnClicked.AddUniqueDynamic(this, &UDialogBankWidget::OnCloseButtonClicked);
}

void UDialogBankWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent    = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
}

void UDialogBankWidget::OnCloseButtonClicked() { OnClose.Broadcast(); }
