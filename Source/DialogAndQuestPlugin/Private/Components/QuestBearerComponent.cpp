
#include "Components/QuestBearerComponent.h"

#include "GameFramework/GameModeBase.h"
#include "Interfaces/DialogGameModeInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DialogAndQuestPluginHelper.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UQuestBearerComponent::UQuestBearerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UQuestBearerComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::RebuildQuestLUT()
{
	KnownQuestDataLUT.Reset();
	for (int32 i = 0; i < KnownQuestData.Num(); ++i)
	{
		KnownQuestDataLUT.Add(KnownQuestData[i].QuestID, i);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::OnRep_KnownQuest()
{
	bool bHasNewQuest = false;

	// Rebuild LUT fully from the replicated array
	TMap<int64, int32> OldLUT = KnownQuestDataLUT;
	RebuildQuestLUT();

	// Detect new quests
	for (const auto& [QuestID, Index] : KnownQuestDataLUT)
	{
		if (!OldLUT.Contains(QuestID))
			bHasNewQuest = true;
	}

	if (bHasNewQuest)
		NewQuestDispatcher.Broadcast();

	KnownQuestDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::Authority_TryProgressQuest(int64 QuestID, AActor* Validator)
{
	if(!GetOwner()->HasAuthority())
		return false;

	if (IDialogGameModeInterface* Gm = Cast<IDialogGameModeInterface>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		return Gm->TryProgressQuest(QuestID, Cast<APlayerController>(GetOwner()), Validator);
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::Server_TryProgressAll_Validate(AActor* Validator)
{
	if (!Validator)
		return false;

	if (!Cast<IQuestGiverInterface>(Validator))
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::Server_TryProgressAll_Implementation(AActor* Validator)
{
	const IQuestGiverInterface* ValidatorGiver = Cast<IQuestGiverInterface>(Validator);
	if (!ValidatorGiver || !ValidatorGiver->GetQuestGiverComponent())
		return;

	for (auto& Data : ValidatorGiver->GetQuestGiverComponent()->GetValidatableQuestSteps())
	{
		Server_TryProgressQuest(Data.Key, Validator);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep, bool SkipReward)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!KnownQuestDataLUT.Contains(QuestMeta.QuestID))
		return;

	FQuestProgressData& QData = KnownQuestData[KnownQuestDataLUT[QuestMeta.QuestID]];

	if (NextQuestStep.QuestSubID != QData.ProgressID || QData.Repeatable)
	{
		QData.CurrentStep.Completed = true;

		if (!SkipReward && QData.CurrentStep.RewardClass != nullptr)
			if (IQuestBearerInterface* SelfBearerInterface = Cast<IQuestBearerInterface>(GetOwner()))
				SelfBearerInterface->GrantReward(QData.CurrentStep.RewardClass);

		if (!QData.Repeatable)
		{
			QData.PreviousStep.Add(MoveTemp(QData.CurrentStep));
			QData.ProgressID = NextQuestStep.QuestSubID;

			FQuestProgressStep NewStepProgress;
			NewStepProgress.Completed = false;
			NewStepProgress.StepDescription = NextQuestStep.StepDescription;
			NewStepProgress.StepTitle = NextQuestStep.StepTitle;
			NewStepProgress.QuestID = NextQuestStep.QuestID;
			NewStepProgress.QuestSubID = NextQuestStep.QuestSubID;
			NewStepProgress.RewardClass = NextQuestStep.RewardClass;
			NewStepProgress.NecessaryItems = NextQuestStep.NecessaryItems;
			NewStepProgress.NecessaryCoins = NextQuestStep.NecessaryCoins;
			NewStepProgress.ItemTurnInDialog = NextQuestStep.ItemTurnInDialog;

			if (NextQuestStep.FinishingStep)
			{
				NewStepProgress.Completed = true;
				QData.State = EQuestState::Completed;
			}
			else
			{
				// If we had been in Achieved state (turned in partial), go back to Accepted
				if (QData.State == EQuestState::Achieved)
					QData.State = EQuestState::Accepted;
			}

			QData.CurrentStep = MoveTemp(NewStepProgress);
			QuestUpdateDispatcher.Broadcast(QData.QuestID, QData.CurrentStep.QuestSubID, QData.State);
		}
	}

	OnRep_KnownQuest();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::AddQuest(const FQuestMetaData& QuestMeta)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!KnownQuestDataLUT.Contains(QuestMeta.QuestID))
	{
		FQuestProgressData NewQuestData;
		NewQuestData.Repeatable = QuestMeta.Repeatable;
		NewQuestData.QuestTitle = QuestMeta.QuestTitle;
		NewQuestData.MentionedDescription = QuestMeta.MentionedDescription;
		NewQuestData.QuestID = QuestMeta.QuestID;
		NewQuestData.ProgressID = 0;
		NewQuestData.State = EQuestState::Accepted;
		if (!QuestMeta.Steps.IsEmpty())
			NewQuestData.CurrentStep = FQuestProgressStep(QuestMeta.Steps[0]);
		KnownQuestData.Add(MoveTemp(NewQuestData));

		RebuildQuestLUT();
		QuestUpdateDispatcher.Broadcast(QuestMeta.QuestID, 0, EQuestState::Accepted);
	}
	else
	{
		// Quest already exists — if it was only Mentioned, transition to Accepted
		FQuestProgressData& Existing = KnownQuestData[KnownQuestDataLUT[QuestMeta.QuestID]];
		if (Existing.State == EQuestState::Mentioned)
		{
			Existing.State = EQuestState::Accepted;
			if (!QuestMeta.Steps.IsEmpty())
				Existing.CurrentStep = FQuestProgressStep(QuestMeta.Steps[0]);
			QuestUpdateDispatcher.Broadcast(QuestMeta.QuestID, 0, EQuestState::Accepted);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::CanValidateStepWithItems(int64 QuestID, int32 StepID, const TArray<int32>& InputItems,
	float InputCoins, TArray<int32>& OutputItems, float& OutputCoins)
{
	OutputItems = InputItems;
	OutputCoins = InputCoins;

	if (!IsAtStep(QuestID, StepID))
		return false;

	const auto& CurrentStep = GetKnownQuest(QuestID).CurrentStep;

	const bool IsExpectingSomething = !CurrentStep.NecessaryItems.IsEmpty() || CurrentStep.NecessaryCoins != 0.f;
	if (!IsExpectingSomething)
		return false;

	TArray<int32> NecessaryItems = CurrentStep.NecessaryItems;
	if (!NecessaryItems.IsEmpty())
	{
		for (const auto& ItemID : InputItems)
		{
			if (const int32 Index = NecessaryItems.Find(ItemID); Index >= 0)
			{
				NecessaryItems.RemoveAt(Index);
				OutputItems.Remove(ItemID);
			}
		}
	}

	const bool GivenItemIsOK = NecessaryItems.IsEmpty();
	float NecessaryCoins = CurrentStep.NecessaryCoins - InputCoins;
	const bool GivenCashIsOk = NecessaryCoins <= 0.f;

	return GivenItemIsOK && GivenCashIsOk;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::Server_TryProgressQuest_Implementation(int64 QuestID, AActor* Validator)
{
	Authority_TryProgressQuest(QuestID, Validator);
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::Server_TryProgressQuest_Validate(int64 QuestID, AActor* Validator)
{
	if (!Validator)
		return false;

	if (QuestID <= 0)
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

const FQuestProgressData& UQuestBearerComponent::GetKnownQuest(int64 QuestID) const
{
	const int32* Index = KnownQuestDataLUT.Find(QuestID);
	check(Index && *Index >= 0 && *Index < KnownQuestData.Num());
	return KnownQuestData[*Index];
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsQuestKnown(int64 QuestID) const
{
	return KnownQuestDataLUT.Contains(QuestID);
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::CanDisplay(int64 QuestID, int32 StepID, EQuestStepConditionType Condition) const
{
	if (!IsQuestKnown(QuestID))
		return false;

	switch (Condition)
	{
	default:
	case EQuestStepConditionType::Equal:        return IsAtStep(QuestID, StepID);
	case EQuestStepConditionType::Lesser:       return IsBeforeStep(QuestID, StepID);
	case EQuestStepConditionType::LesserEqual:  return IsBeforeOrAtStep(QuestID, StepID);
	case EQuestStepConditionType::Greater:       return IsPastStep(QuestID, StepID);
	case EQuestStepConditionType::GreaterEqual: return IsAtOrPastStep(QuestID, StepID);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::CanValidate(int64 QuestID, int32 StepID) const
{
	if (StepID == 0)
		return !IsQuestKnown(QuestID) || GetKnownQuest(QuestID).State == EQuestState::Mentioned;

	if (!IsQuestKnown(QuestID))
		return false;

	if (IsPastStep(QuestID, StepID))
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsBeforeStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).QuestID > 0 && GetKnownQuest(QuestID).ProgressID < StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsBeforeOrAtStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).QuestID > 0 && GetKnownQuest(QuestID).ProgressID <= StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsPastStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).ProgressID > StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsAtOrPastStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).ProgressID >= StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsAtStep(int64 QuestID, int32 StepID) const
{
	if (!IsQuestKnown(QuestID))
		return false;

	return GetKnownQuest(QuestID).ProgressID == StepID;
}

//----------------------------------------------------------------------------------------------------------------------
// State machine transitions
//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::MentionQuest(int64 QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (KnownQuestDataLUT.Contains(QuestID))
		return; // Already known — can't re-mention

	IDialogGameModeInterface* GM = Cast<IDialogGameModeInterface>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GetMainQuestComponent())
		return;

	const FQuestMetaData& QuestData = GM->GetMainQuestComponent()->GetQuestData(QuestID);
	if (QuestData.QuestID == 0)
		return;

	FQuestProgressData NewQuestData;
	NewQuestData.QuestID = QuestData.QuestID;
	NewQuestData.QuestTitle = QuestData.QuestTitle;
	NewQuestData.MentionedDescription = QuestData.MentionedDescription;
	NewQuestData.Repeatable = QuestData.Repeatable;
	NewQuestData.State = EQuestState::Mentioned;
	NewQuestData.ProgressID = -1; // No steps yet
	KnownQuestData.Add(MoveTemp(NewQuestData));

	RebuildQuestLUT();
	QuestUpdateDispatcher.Broadcast(QuestID, -1, EQuestState::Mentioned);
	OnRep_KnownQuest();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::AchieveQuest(int64 QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!KnownQuestDataLUT.Contains(QuestID))
		return;

	FQuestProgressData& QData = KnownQuestData[KnownQuestDataLUT[QuestID]];
	if (QData.State != EQuestState::Accepted)
	{
		UDialogAndQuestPluginHelper::Warning(FString::Printf(TEXT("Cannot achieve quest %lld: not in Accepted state (current: %d)"), QuestID, static_cast<uint8>(QData.State)));
		return;
	}

	QData.State = EQuestState::Achieved;
	QuestUpdateDispatcher.Broadcast(QuestID, QData.ProgressID, EQuestState::Achieved);
	OnRep_KnownQuest();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::CompleteQuest(int64 QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!KnownQuestDataLUT.Contains(QuestID))
		return;

	FQuestProgressData& QData = KnownQuestData[KnownQuestDataLUT[QuestID]];
	if (QData.State != EQuestState::Achieved && QData.State != EQuestState::Accepted)
	{
		UDialogAndQuestPluginHelper::Warning(FString::Printf(TEXT("Cannot complete quest %lld: not in Achieved/Accepted state (current: %d)"), QuestID, static_cast<uint8>(QData.State)));
		return;
	}

	QData.State = EQuestState::Completed;
	QData.CurrentStep.Completed = true;
	QuestUpdateDispatcher.Broadcast(QuestID, QData.ProgressID, EQuestState::Completed);
	OnRep_KnownQuest();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::BotchQuest(int64 QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!KnownQuestDataLUT.Contains(QuestID))
		return;

	FQuestProgressData& QData = KnownQuestData[KnownQuestDataLUT[QuestID]];
	if (QData.IsTerminal())
	{
		UDialogAndQuestPluginHelper::Warning(FString::Printf(TEXT("Cannot botch quest %lld: already terminal (current: %d)"), QuestID, static_cast<uint8>(QData.State)));
		return;
	}

	QData.State = EQuestState::Botched;
	QuestUpdateDispatcher.Broadcast(QuestID, QData.ProgressID, EQuestState::Botched);
	OnRep_KnownQuest();
}

//----------------------------------------------------------------------------------------------------------------------

EQuestState UQuestBearerComponent::GetQuestState(int64 QuestID) const
{
	if (!IsQuestKnown(QuestID))
		return EQuestState::Unknown;

	return GetKnownQuest(QuestID).State;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::AuthorityAddQuest(int64 QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	IDialogGameModeInterface* IDialogInterface = Cast<IDialogGameModeInterface>(GetWorld()->GetAuthGameMode());
	check(IDialogInterface);

	const FQuestMetaData& QuestData = IDialogInterface->GetMainQuestComponent()->GetQuestData(QuestID);
	if (QuestData.QuestID == 0)
		return;

	AddQuest(QuestData);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::AuthoritySetupQuestData(int64 QuestID, int32 StepID, EQuestState InitialState)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	IDialogGameModeInterface* GM = Cast<IDialogGameModeInterface>(GetWorld()->GetAuthGameMode());
	check(GM);

	UQuestMainComponent* MQC = GM->GetMainQuestComponent();
	if (!MQC)
		return;

	const FQuestMetaData& Meta = MQC->GetQuestData(QuestID);
	if (Meta.QuestID == 0)
		return;

	// Handle Mentioned state — just create a mention entry, don't set up steps
	if (InitialState == EQuestState::Mentioned)
	{
		MentionQuest(QuestID);
		return;
	}

	AuthorityAddQuest(QuestID);
	if (KnownQuestDataLUT.Contains(QuestID))
	{
		for (int32 CurrentStepID = 0; CurrentStepID < StepID; ++CurrentStepID)
		{
			ProgressQuest(Meta, MQC->FindNextStep(Meta, CurrentStepID), true);
		}

		// Apply the requested initial state
		if (InitialState != EQuestState::Accepted)
		{
			FQuestProgressData& QData = KnownQuestData[KnownQuestDataLUT[QuestID]];
			QData.State = InitialState;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::TryProgressQuest(int64 QuestID, AActor* Validator)
{
	Server_TryProgressQuest(QuestID, Validator);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::TryProgressAll(AActor* Validator)
{
	Server_TryProgressAll(Validator);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UQuestBearerComponent, KnownQuestData, COND_OwnerOnly);
}
