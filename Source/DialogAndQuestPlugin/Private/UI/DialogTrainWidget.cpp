#include "UI/DialogTrainWidget.h"
#include "Components/Button.h"

void UDialogTrainWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
		CloseButton->OnClicked.AddUniqueDynamic(this, &UDialogTrainWidget::OnCloseButtonClicked);
}

void UDialogTrainWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent    = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
}

void UDialogTrainWidget::OnCloseButtonClicked() { OnClose.Broadcast(); }
