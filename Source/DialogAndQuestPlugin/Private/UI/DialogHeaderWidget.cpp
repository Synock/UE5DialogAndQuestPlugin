#include "UI/DialogHeaderWidget.h"
#include "Components/Button.h"

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button click events - buttons are optional so check validity first
	if (TradeButton)
	{
		TradeButton->OnClicked.AddDynamic(this, &UDialogHeaderWidget::OnTradeButtonClicked);
	}

	if (GiveButton)
	{
		GiveButton->OnClicked.AddDynamic(this, &UDialogHeaderWidget::OnGiveButtonClicked);
	}

	if (TrainButton)
	{
		TrainButton->OnClicked.AddDynamic(this, &UDialogHeaderWidget::OnTrainButtonClicked);
	}

	if (BankButton)
	{
		BankButton->OnClicked.AddDynamic(this, &UDialogHeaderWidget::OnBankButtonClicked);
	}

	if (RepairButton)
	{
		RepairButton->OnClicked.AddDynamic(this, &UDialogHeaderWidget::OnRepairButtonClicked);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::InitDialog(UDialogWindow* InputParentDialog)
{
	ParentDialog = InputParentDialog;
	DialogComponent = InputParentDialog->GetDialogComponent();
	check(DialogComponent);
	SetDialogName(DialogComponent->GetDialogName());
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::ConfigureButtons(bool bCanTrade, bool bCanGive, bool bCanTrain, bool bCanBank, bool bCanRepair)
{
	if (TradeButton)
	{
		TradeButton->SetVisibility(bCanTrade ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (GiveButton)
	{
		GiveButton->SetVisibility(bCanGive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (TrainButton)
	{
		TrainButton->SetVisibility(bCanTrain ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (BankButton)
	{
		BankButton->SetVisibility(bCanBank ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (RepairButton)
	{
		RepairButton->SetVisibility(bCanRepair ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::OnTradeButtonClicked()
{
	if (ParentDialog)
	{
		ParentDialog->DisplayTradeWidget();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::OnGiveButtonClicked()
{
	if (ParentDialog)
	{
		ParentDialog->DisplayGiveWidget();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::OnTrainButtonClicked()
{
	if (ParentDialog)
	{
		ParentDialog->DisplayTrainDialogWidget();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::OnBankButtonClicked()
{
	if (ParentDialog)
	{
		ParentDialog->DisplayBankDialogWidget();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::OnRepairButtonClicked()
{
	if (ParentDialog)
	{
		ParentDialog->DisplayRepairDialogWidget();
	}
}

