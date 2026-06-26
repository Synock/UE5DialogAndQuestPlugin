#include "UI/DialogFooterWidget.h"
#include "Components/Button.h"

void UDialogFooterWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CloseButton)
		CloseButton->OnClicked.AddUniqueDynamic(this, &UDialogFooterWidget::OnCloseButtonClicked);
}

void UDialogFooterWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent    = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
}

void UDialogFooterWidget::OnCloseButtonClicked()
{
	if (ParentDialogObject)
		IDialogWindowInterface::Execute_CloseWindow(ParentDialogObject.Get());
}
