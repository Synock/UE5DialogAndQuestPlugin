#include "UI/DialogWindow.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Interfaces/DialogInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "UI/DialogBankWidget.h"
#include "UI/DialogFooterWidget.h"
#include "UI/DialogGiveWidget.h"
#include "UI/DialogHeaderWidget.h"
#include "UI/DialogRepairWidget.h"
#include "UI/DialogTextWidget.h"
#include "UI/DialogTopicWidget.h"
#include "UI/DialogTradeWidget.h"
#include "UI/DialogTrainWidget.h"

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::NativeConstruct()
{
	Super::NativeConstruct();

	// Validate required widgets are bound
	if (!Footer || !Header || !TopicList || !TopicText || !WidgetSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("UDialogWindow: Required widgets not bound. Check Blueprint widget names match BindWidget properties."));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayGiveWidget()
{
	OnGive.Broadcast();
	if (GiveWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(GiveWidgetPointer);
		TopicList->SetIsEnabled(false);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayTradeWidget()
{
	OnTrade.Broadcast();
	if (TradeWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(TradeWidgetPointer);
		TopicList->SetIsEnabled(false);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayMainDialogWidget()
{
	if (TopicText)
	{
		WidgetSwitcher->SetActiveWidget(TopicText);
	}

	TopicList->SetIsEnabled(true);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayTrainDialogWidget()
{
	OnTrain.Broadcast();
	if (TrainWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(TrainWidgetPointer);
		TopicList->SetIsEnabled(false);
		TrainWidgetPointer->DoOnDisplay();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayBankDialogWidget()
{
	OnBank.Broadcast();
	if (BankWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(BankWidgetPointer);
		TopicList->SetIsEnabled(false);
		BankWidgetPointer->DoOnDisplay();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayRepairDialogWidget()
{
	OnRepair.Broadcast();
	if (RepairWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(RepairWidgetPointer);
		TopicList->SetIsEnabled(false);
		RepairWidgetPointer->DoOnDisplay();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::AddTradeWidget(UDialogTradeWidget* TradeWidget)
{
	TradeWidgetPointer = TradeWidget;
	WidgetSwitcher->AddChild(TradeWidgetPointer);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::AddGiveWidget(UDialogGiveWidget* GiveWidget)
{
	GiveWidgetPointer = GiveWidget;
	WidgetSwitcher->AddChild(GiveWidgetPointer);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::AddTrainWidget(UDialogTrainWidget* TrainWidget)
{
	TrainWidgetPointer = TrainWidget;
	WidgetSwitcher->AddChild(TrainWidgetPointer);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::AddBankWidget(UDialogBankWidget* BankWidget)
{
	BankWidgetPointer = BankWidget;
	WidgetSwitcher->AddChild(BankWidgetPointer);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::AddRepairWidget(UDialogRepairWidget* RepairWidget)
{
	RepairWidgetPointer = RepairWidget;
	WidgetSwitcher->AddChild(RepairWidgetPointer);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::InitDialogWindow(UDialogComponent* InputDialogComponent, AActor* ActorDialog)
{
	check(InputDialogComponent);

	DialogActor = ActorDialog;


	IDialogInterface* DialogActorInterface = Cast<IDialogInterface>(DialogActor);
	if (DialogActor && DialogActorInterface)
	{
		RelationValue = DialogActorInterface->GetRelation(GetOwningPlayerPawn());
		RelationString = DialogActorInterface->GetRelationString(RelationValue);
		DialogComponent = DialogActorInterface->GetDialogComponent();
	}
	else
	{
		DialogComponent = InputDialogComponent;
		PostInitRelation();
	}

	Footer->InitDialog(this);
	Header->InitDialog(this);
	TopicText->InitDialog(this);
	TopicList->InitDialog(this);

	// Configure header buttons based on NPC capabilities
	if (Header && DialogActorInterface)
	{
		Header->ConfigureButtons(
			DialogActorInterface->CanTrade(),
			DialogActorInterface->CanGive(),
			DialogActorInterface->CanTrain(),
			DialogActorInterface->CanBank(),
			DialogActorInterface->CanRepair()
		);
	}

	Header->SetRelationValue(RelationValue);
	Header->SetRelationString(RelationString);

	Header->SetDialogName(DialogActorInterface->GetCharacterNameForDialog().ToString());
	TopicText->ClearList();
	TopicList->UpdateTopicData();

	if (RelationValue >= DialogComponent->GetGreetingLimit())
	{
		TopicText->AddEmptyTopicData(DialogComponent->GetGoodGreeting());
	}
	else
	{
		TopicText->AddEmptyTopicData(DialogComponent->GetBadGreeting());
	}


	if (IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer()))
	{
		BearerInterface->GetQuestBearerComponent()->KnownQuestDispatcher.AddDynamic(
			this, &UDialogWindow::DisplayJournalUpdate);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayJournalUpdate()
{
	RefreshDialogOptions();
	TopicText->AddEmptyTopicData("<Italic>Your quest journal has been updated.</>");
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayDialogTopic(int64 ID)
{
	TopicText->AddTopicText(ID);
	//also try to update possible quest advancement here?

	if (IQuestGiverInterface* GiverInterface = Cast<IQuestGiverInterface>(DialogActor))
	{
		const FDialogTopicStruct& Topic = DialogComponent->GetDialogTopic(ID);

		if (IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer()))
		{
			if (Topic.QuestRelation.QuestID != 0)
			{
				for (auto& StepData : Topic.QuestRelation.Steps)
				{
					if (BearerInterface->CanValidate(Topic.QuestRelation.QuestID, StepData))
					{
						BearerInterface->TryProgressQuest(
							Topic.QuestRelation.QuestID, DialogActor);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayDialogTopicFromString(const FString& ID)
{
	DisplayDialogTopic(DialogComponent->GetDialogTopicID(ID));
}


//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayPlainString(const FString& PlainString)
{
	TopicText->AddEmptyTopicData(PlainString);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::CloseWindow()
{
	if (IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer()))
	{
		BearerInterface->GetQuestBearerComponent()->KnownQuestDispatcher.RemoveAll(this);
	}

	DisplayMainDialogWidget();
	OnExit.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::RefreshDialogOptions()
{
	TopicList->UpdateTopicData();
	TopicText->ReprocessTopicLinks();
}
