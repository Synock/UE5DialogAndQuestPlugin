#include "UI/DialogTradeWidget.h"
#include "Components/Button.h"

void UDialogTradeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
		CloseButton->OnClicked.AddUniqueDynamic(this, &UDialogTradeWidget::OnCloseButtonClicked);
}

void UDialogTradeWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent    = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
}

void UDialogTradeWidget::OnCloseButtonClicked() { OnClose.Broadcast(); }
