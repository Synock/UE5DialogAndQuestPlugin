// Copyright 2022 Maximilien (Synock) Guislain


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
#include "UI/DialogTextWidget.h"
#include "UI/DialogTopicWidget.h"
#include "UI/DialogTradeWidget.h"
#include "UI/DialogTrainWidget.h"

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

	/*
	TradeButton->OnClicked.AddDynamic(this,&UDialogWindow::DisplayTradeWidget);
	GiveButton->OnClicked.AddDynamic(this,&UDialogWindow::DisplayGiveWidget);
*/
	if (TradeButton && DialogActorInterface->CanTrade())
	{
		TradeButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		TradeButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (GiveButton && DialogActorInterface->CanGive())
	{
		GiveButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GiveButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TrainButton && DialogActorInterface->CanTrain())
	{
		TrainButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		TrainButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (BankButton && DialogActorInterface->CanBank())
	{
		BankButton->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		BankButton->SetVisibility(ESlateVisibility::Collapsed);
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

	OnExit.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::RefreshDialogOptions()
{
	TopicList->UpdateTopicData();
	TopicText->ReprocessTopicLinks();
}
