#include "UI/DialogWindow.h"

#include "Components/WidgetSwitcher.h"
#include "Engine/AssetManager.h"
#include "Interfaces/DialogConsequenceInterface.h"
#include "Interfaces/DialogInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Sound/SoundBase.h"
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

	if (!Footer || !Header || !TopicList || !TopicText || !WidgetSwitcher)
	{
		UE_LOG(LogTemp, Error, TEXT("UDialogWindow: Required widgets not bound. Check Blueprint widget names match BindWidget properties."));
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::AddTradeWidget(UDialogTradeWidget* TradeWidget)
{
	TradeWidgetPointer = TradeWidget;
	WidgetSwitcher->AddChild(TradeWidgetPointer);
}

void UDialogWindow::AddGiveWidget(UDialogGiveWidget* GiveWidget)
{
	GiveWidgetPointer = GiveWidget;
	WidgetSwitcher->AddChild(GiveWidgetPointer);
}

void UDialogWindow::AddTrainWidget(UDialogTrainWidget* TrainWidget)
{
	TrainWidgetPointer = TrainWidget;
	WidgetSwitcher->AddChild(TrainWidgetPointer);
}

void UDialogWindow::AddBankWidget(UDialogBankWidget* BankWidget)
{
	BankWidgetPointer = BankWidget;
	WidgetSwitcher->AddChild(BankWidgetPointer);
}

void UDialogWindow::AddRepairWidget(UDialogRepairWidget* RepairWidget)
{
	RepairWidgetPointer = RepairWidget;
	WidgetSwitcher->AddChild(RepairWidgetPointer);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::InitDialogWindow_Implementation(UDialogComponent* InputDialogComponent, AActor* ActorDialog)
{
	check(InputDialogComponent);
	InvalidatePendingGreetingVoiceover();

	DialogActor = ActorDialog;

	IDialogInterface* DialogActorInterface = Cast<IDialogInterface>(DialogActor);
	if (DialogActor && DialogActorInterface)
	{
		RelationValue  = DialogActorInterface->GetRelation(GetOwningPlayerPawn());
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
	DisplayMainDialogWidget_Implementation();

	if (Header && DialogActorInterface)
	{
		Header->ConfigureButtons(
			DialogActorInterface->CanTrade(),
			DialogActorInterface->CanGive(),
			DialogActorInterface->CanTrainPlayer(GetOwningPlayerPawn()),
			DialogActorInterface->CanBank(),
			DialogActorInterface->CanRepair()
		);
		// Disable service buttons when relation is below the NPC's greeting threshold.
		// Buttons remain visible but greyed-out so the player can see what they're missing.
		const bool bRelationOK = RelationValue >= DialogComponent->GetGreetingLimit();
		Header->SetServiceButtonsEnabled(bRelationOK);
		Header->SetRelationValue(RelationValue);
		Header->SetRelationString(RelationString);
		Header->SetDialogName(DialogActorInterface->GetCharacterNameForDialog().ToString());
	}

	TopicText->ClearList();
	TopicList->UpdateTopicData();

	const bool bGoodGreeting = RelationValue >= DialogComponent->GetGreetingLimit();
	if (bGoodGreeting)
		TopicText->AddEmptyTopicData(DialogComponent->GetGoodGreeting().ToString());
	else
	{
		// Allow the NPC (or any IDialogInterface implementor) to supply a context-aware
		// Falls back to the static BadGreeting in the DialogComponent when not overridden.
		FText ChosenBadGreeting = DialogActorInterface
			? DialogActorInterface->GetContextualBadGreeting(RelationValue, GetOwningPlayerPawn())
			: DialogComponent->GetBadGreeting();
		TopicText->AddEmptyTopicData(ChosenBadGreeting.ToString());
	}

	const TSoftObjectPtr<USoundBase> GreetingVO = bGoodGreeting
		? DialogComponent->GetGoodGreetingVoiceover()
		: DialogComponent->GetBadGreetingVoiceover();

	RequestGreetingVoiceover(GreetingVO);

	if (IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer()))
	{
		BearerInterface->GetQuestBearerComponent()->KnownQuestDispatcher.AddUniqueDynamic(
			this, &UDialogWindow::DisplayJournalUpdate);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::CloseWindow_Implementation()
{
	InvalidatePendingGreetingVoiceover();

	if (DialogComponent)
		DialogComponent->OnVoiceoverStop.Broadcast();

	if (IQuestBearerInterface* BearerInterface = Cast<IQuestBearerInterface>(GetOwningPlayer()))
		BearerInterface->GetQuestBearerComponent()->KnownQuestDispatcher.RemoveAll(this);

	DisplayMainDialogWidget_Implementation();
	OnExit.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::RefreshDialogOptions_Implementation()
{
	TopicList->UpdateTopicData();
	TopicText->ReprocessTopicLinks();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayDialogTopic_Implementation(int64 ID)
{
	const FDialogTopicStruct* TopicPtr = DialogComponent->GetDialogTopicSafe(ID);
	if (!TopicPtr)
		return;

	const FDialogTopicStruct Topic = *TopicPtr;
	TopicPtr = nullptr;

	DialogComponent->OnVoiceoverStop.Broadcast();
	TopicText->AddTopicText(ID);

	if (Topic.Consequence.HasConsequence())
	{
		if (IDialogConsequenceInterface* Handler = Cast<IDialogConsequenceInterface>(GetOwningPlayer()))
			Handler->HandleDialogConsequence(Topic.Id, DialogActor.Get());
	}

	if (Topic.TopicCondition.bConsumeOnUse)
		DialogComponent->ConsumeTopicByID(ID);

	if (!Topic.VoiceoverCue.IsNull())
	{
		TWeakObjectPtr<UDialogComponent> WeakComp(DialogComponent.Get());
		const float Dur = Topic.VoiceoverDuration;
		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			Topic.VoiceoverCue.ToSoftObjectPath(),
			FStreamableDelegate::CreateWeakLambda(this,
				[WeakComp, SoftCue = Topic.VoiceoverCue, Dur]()
				{
					if (UDialogComponent* DC = WeakComp.Get())
						if (USoundBase* Sound = SoftCue.Get())
							DC->OnVoiceoverRequested.Broadcast(Sound, Dur);
				})
		);
	}
	if (!Topic.VoiceoverEventName.IsNone())
		DialogComponent->OnMiddlewareVoiceoverRequested.Broadcast(Topic.VoiceoverEventName);


	RefreshDialogOptions_Implementation();
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayDialogTopicFromString_Implementation(const FString& ID)
{
	DisplayDialogTopic_Implementation(DialogComponent->GetDialogTopicID(ID));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayPlainString_Implementation(const FString& PlainString)
{
	TopicText->AddEmptyTopicData(PlainString);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayJournalUpdate()
{
	RefreshDialogOptions_Implementation();
	TopicText->AddEmptyTopicData(TEXT("<Italic>Your quest journal has been updated.</>"));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogWindow::DisplayMainDialogWidget_Implementation()
{
	if (TopicText)
		WidgetSwitcher->SetActiveWidget(TopicText);
	TopicList->SetIsEnabled(true);
}

void UDialogWindow::DisplayTradeWidget_Implementation()
{
	StopVoiceoverForServiceTab();
	OnTrade.Broadcast();
	if (TradeWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(TradeWidgetPointer);
		TopicList->SetIsEnabled(false);
	}
}

void UDialogWindow::DisplayGiveWidget_Implementation()
{
	StopVoiceoverForServiceTab();
	OnGive.Broadcast();
	if (GiveWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(GiveWidgetPointer);
		TopicList->SetIsEnabled(false);
	}
}

void UDialogWindow::DisplayTrainDialogWidget_Implementation()
{
	StopVoiceoverForServiceTab();
	OnTrain.Broadcast();
	if (TrainWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(TrainWidgetPointer);
		TopicList->SetIsEnabled(false);
		TrainWidgetPointer->DoOnDisplay();
	}
}

void UDialogWindow::DisplayBankDialogWidget_Implementation()
{
	StopVoiceoverForServiceTab();
	OnBank.Broadcast();
	if (BankWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(BankWidgetPointer);
		TopicList->SetIsEnabled(false);
		BankWidgetPointer->DoOnDisplay();
	}
}

void UDialogWindow::DisplayRepairDialogWidget_Implementation()
{
	StopVoiceoverForServiceTab();
	OnRepair.Broadcast();
	if (RepairWidgetPointer)
	{
		WidgetSwitcher->SetActiveWidget(RepairWidgetPointer);
		TopicList->SetIsEnabled(false);
		RepairWidgetPointer->DoOnDisplay();
	}
}

void UDialogWindow::InvalidatePendingGreetingVoiceover()
{
	++GreetingVoiceoverRequestSerial;
}

void UDialogWindow::StopVoiceoverForServiceTab()
{
	InvalidatePendingGreetingVoiceover();

	if (DialogComponent)
	{
		DialogComponent->OnVoiceoverStop.Broadcast();
	}
}

void UDialogWindow::RequestGreetingVoiceover(const TSoftObjectPtr<USoundBase>& GreetingVoiceover)
{
	if (GreetingVoiceover.IsNull())
	{
		return;
	}

	const int32 RequestSerial = GreetingVoiceoverRequestSerial;
	TWeakObjectPtr<UDialogComponent> WeakComp(DialogComponent.Get());
	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		GreetingVoiceover.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this,
			[WeakComp, SoftVO = GreetingVoiceover, WeakWindow = TWeakObjectPtr<UDialogWindow>(this), RequestSerial]()
			{
				UDialogWindow* Window = WeakWindow.Get();
				if (!Window || !Window->ShouldPlayPendingGreetingVoiceover(RequestSerial))
				{
					return;
				}

				if (UDialogComponent* DC = WeakComp.Get())
				{
					if (USoundBase* Sound = SoftVO.Get())
					{
						DC->OnVoiceoverRequested.Broadcast(Sound, 0.f);
					}
				}
			})
	);
}

bool UDialogWindow::ShouldPlayPendingGreetingVoiceover(int32 RequestSerial) const
{
	return GreetingVoiceoverRequestSerial == RequestSerial &&
		DialogComponent &&
		WidgetSwitcher &&
		TopicText &&
		WidgetSwitcher->GetActiveWidget() == TopicText;
}
