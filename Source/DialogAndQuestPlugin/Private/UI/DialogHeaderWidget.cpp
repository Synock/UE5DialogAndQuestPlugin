#include "UI/DialogHeaderWidget.h"
#include "Components/Button.h"
#include "Components/DialogComponent.h"
#include "Components/TextBlock.h"

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TradeButton)  TradeButton->OnClicked.AddDynamic(this,  &UDialogHeaderWidget::OnTradeButtonClicked);
	if (GiveButton)   GiveButton->OnClicked.AddDynamic(this,   &UDialogHeaderWidget::OnGiveButtonClicked);
	if (TrainButton)  TrainButton->OnClicked.AddDynamic(this,  &UDialogHeaderWidget::OnTrainButtonClicked);
	if (BankButton)   BankButton->OnClicked.AddDynamic(this,   &UDialogHeaderWidget::OnBankButtonClicked);
	if (RepairButton) RepairButton->OnClicked.AddDynamic(this, &UDialogHeaderWidget::OnRepairButtonClicked);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::InitDialog(UObject* InputParentDialog)
{
	ParentDialogObject = InputParentDialog;
	DialogComponent = IDialogWindowInterface::Execute_GetDialogComponent(InputParentDialog);
	check(DialogComponent);
	SetDialogName(DialogComponent->GetDialogName());
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::SetDialogName_Implementation(const FString& Name)
{
	if (NameText)
		NameText->SetText(FText::FromString(Name));
}

void UDialogHeaderWidget::SetRelationValue_Implementation(float RelationFloat)
{
	if (RelationValueText)
		RelationValueText->SetText(FText::AsNumber(RelationFloat));
}

void UDialogHeaderWidget::SetRelationString_Implementation(const FString& Name)
{
	if (RelationStringText)
		RelationStringText->SetText(FText::FromString(Name));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::ConfigureButtons(bool bCanTrade, bool bCanGive, bool bCanTrain, bool bCanBank, bool bCanRepair)
{
	auto SetVis = [](UButton* Btn, bool bShow)
	{
		if (Btn) Btn->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	};
	SetVis(TradeButton,  bCanTrade);
	SetVis(GiveButton,   bCanGive);
	SetVis(TrainButton,  bCanTrain);
	SetVis(BankButton,   bCanBank);
	SetVis(RepairButton, bCanRepair);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogHeaderWidget::OnTradeButtonClicked()  { IDialogWindowInterface::Execute_DisplayTradeWidget(ParentDialogObject.Get()); }
void UDialogHeaderWidget::OnGiveButtonClicked()   { IDialogWindowInterface::Execute_DisplayGiveWidget(ParentDialogObject.Get()); }
void UDialogHeaderWidget::OnTrainButtonClicked()  { IDialogWindowInterface::Execute_DisplayTrainDialogWidget(ParentDialogObject.Get()); }
void UDialogHeaderWidget::OnBankButtonClicked()   { IDialogWindowInterface::Execute_DisplayBankDialogWidget(ParentDialogObject.Get()); }
void UDialogHeaderWidget::OnRepairButtonClicked() { IDialogWindowInterface::Execute_DisplayRepairDialogWidget(ParentDialogObject.Get()); }
