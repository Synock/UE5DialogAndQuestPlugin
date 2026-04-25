#include "Interfaces/DialogWindowInterface.h"

#include "Blueprint/UserWidget.h"
#include "Components/DialogComponent.h"
#include "Engine/AssetManager.h"
#include "Interfaces/DialogConsequenceInterface.h"
#include "Interfaces/DialogDisplayInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// ---- InitDialogWindow -------------------------------------------------------

void IDialogWindowInterface::InitDialogWindow_Implementation(UDialogComponent* InputDialogComponent, AActor* ActorDialog)
{
	// The base implementation performs no storage — concrete classes override this
	// to store the component/actor and call Execute_OnDialogOpened.
	// Provided as a non-crashing fallback only.
}

// ---- CloseWindow ------------------------------------------------------------

void IDialogWindowInterface::CloseWindow_Implementation()
{
	// Stop any ongoing voiceover.
	if (UDialogComponent* Comp = Execute_GetDialogComponent(_getUObject()))
		Comp->OnVoiceoverStop.Broadcast();

	Execute_OnDialogClosed(_getUObject());
}

// ---- RefreshDialogOptions ---------------------------------------------------

void IDialogWindowInterface::RefreshDialogOptions_Implementation()
{
	UUserWidget* Widget = Cast<UUserWidget>(_getUObject());
	if (!Widget)
		return;

	UDialogComponent* Comp = Execute_GetDialogComponent(_getUObject());
	AActor* Actor           = Execute_GetDialogActor(_getUObject());
	APlayerController* PC   = Widget->GetOwningPlayer();
	if (!Comp || !PC)
		return;

	TArray<FDialogTopicStruct> Visible;
	for (const FDialogTopicStruct& Topic : Comp->GetAllDialogTopic())
	{
		if (Topic.TopicCondition.VerifyCondition(Actor, PC))
			Visible.Add(Topic);
	}

	Execute_OnTopicListUpdated(_getUObject(), Visible);
}

// ---- DisplayDialogTopic -----------------------------------------------------

void IDialogWindowInterface::DisplayDialogTopic_Implementation(int64 ID)
{
	UObject* Self         = _getUObject();
	UUserWidget* Widget   = Cast<UUserWidget>(Self);
	if (!Widget)
		return;

	UDialogComponent* Comp = Execute_GetDialogComponent(Self);
	AActor* Actor          = Execute_GetDialogActor(Self);
	if (!Comp)
		return;

	const FDialogTopicStruct* TopicPtr = Comp->GetDialogTopicSafe(ID);
	if (!TopicPtr)
		return;

	// Copy by value — ConsumeTopicByID below may remove the TMap entry.
	const FDialogTopicStruct Topic = *TopicPtr;
	TopicPtr = nullptr;

	// Stop any ongoing voiceover.
	Comp->OnVoiceoverStop.Broadcast();

	// Build the display struct with processed hyperlinks.
	APlayerController* PC = Widget->GetOwningPlayer();
	FDialogTextData TextData;
	TextData.Id              = Topic.Id;
	TextData.TopicName       = Topic.Topic;
	TextData.TopicText       = Comp->ParseTextHyperlink(Topic.TopicText.ToString(), Actor, PC);
	TextData.VoiceoverCue    = Topic.VoiceoverCue;
	TextData.VoiceoverDuration = Topic.VoiceoverDuration;

	// Notify the implementing class (Blueprint fills in the visual logic).
	Execute_OnTopicTextReady(Self, TextData);

	// Fire dialog consequence server-side (via IDialogConsequenceInterface on PlayerController).
	if (Topic.Consequence.HasConsequence())
	{
		if (IDialogConsequenceInterface* Handler = Cast<IDialogConsequenceInterface>(PC))
			Handler->HandleDialogConsequence(Topic.Id, Actor);
	}

	// Consume-on-use.
	if (Topic.TopicCondition.bConsumeOnUse)
		Comp->ConsumeTopicByID(ID);

	// Voiceover — async load to avoid a game-thread hitch.
	if (!Topic.VoiceoverCue.IsNull())
	{
		TWeakObjectPtr<UDialogComponent> WeakComp(Comp);
		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			Topic.VoiceoverCue.ToSoftObjectPath(),
			FStreamableDelegate::CreateWeakLambda(Widget,
				[WeakComp, SoftVO = Topic.VoiceoverCue, Dur = Topic.VoiceoverDuration]()
				{
					if (UDialogComponent* DC = WeakComp.Get())
						if (USoundBase* Sound = SoftVO.Get())
							DC->OnVoiceoverRequested.Broadcast(Sound, Dur);
				})
		);
	}
	if (!Topic.VoiceoverEventName.IsNone())
		Comp->OnMiddlewareVoiceoverRequested.Broadcast(Topic.VoiceoverEventName);

	// Legacy quest-relation (backward compat with QuestRelation field on topic structs).
	if (IQuestGiverInterface* GiverIF = Cast<IQuestGiverInterface>(Actor))
	{
		if (IQuestBearerInterface* BearerIF = Cast<IQuestBearerInterface>(PC))
		{
			if (Topic.QuestRelation.QuestID != 0)
			{
				for (const auto& StepData : Topic.QuestRelation.Steps)
				{
					if (BearerIF->CanValidate(Topic.QuestRelation.QuestID, StepData))
						BearerIF->TryProgressQuest(Topic.QuestRelation.QuestID, Actor);
				}
			}
		}
	}

	Execute_RefreshDialogOptions(Self);
}

// ---- DisplayDialogTopicFromString -------------------------------------------

void IDialogWindowInterface::DisplayDialogTopicFromString_Implementation(const FString& ID)
{
	UObject* Self = _getUObject();
	if (UDialogComponent* Comp = Execute_GetDialogComponent(Self))
		Execute_DisplayDialogTopic(Self, Comp->GetDialogTopicID(ID));
}

// ---- DisplayPlainString -----------------------------------------------------

void IDialogWindowInterface::DisplayPlainString_Implementation(const FString& PlainString)
{
	UObject* Self       = _getUObject();
	UUserWidget* Widget = Cast<UUserWidget>(Self);
	if (!Widget)
		return;

	APlayerController* PC  = Widget->GetOwningPlayer();
	UDialogComponent* Comp = Execute_GetDialogComponent(Self);
	AActor* Actor          = Execute_GetDialogActor(Self);

	FString Processed = PlainString;
	if (const IDialogDisplayInterface* Displayer = Cast<IDialogDisplayInterface>(PC))
		Processed = Displayer->ProcessScriptedFunction(Processed, Widget->GetWorld());
	if (Comp && Actor && PC)
		Processed = Comp->ParseTextHyperlink(Processed, Actor, PC);

	FDialogTextData TextData;
	TextData.Id       = 0;
	TextData.TopicText = Processed;
	Execute_OnTopicTextReady(Self, TextData);
}

