#include "Components/QuestBearerComponent.h"

#include "GameFramework/GameModeBase.h"
#include "Components/QuestMainComponent.h"
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

void UQuestBearerComponent::ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep,
                                           bool SkipReward, bool bSilent)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!KnownQuestDataLUT.Contains(QuestMeta.QuestID))
		return;

	FQuestProgressData& QData = KnownQuestData[KnownQuestDataLUT[QuestMeta.QuestID]];

	// Never mutate a terminal quest — Completed and Botched are irreversible.
	if (QData.IsTerminal())
		return;

	// Warn when we receive a sentinel as the next step but the current step is not the
	// FinishingStep. This indicates malformed quest data (no FinishingStep marker set).
	if (UQuestMainComponent::IsStepSentinel(NextQuestStep) && !QData.CurrentStep.FinishingStep)
	{
		UDialogAndQuestPluginHelper::Warning(FString::Printf(
			TEXT("ProgressQuest: QID=%lld reached end of step chain at SubID=%d without a FinishingStep — quest will complete incorrectly; fix the quest asset"),
			QuestMeta.QuestID, QData.CurrentStep.QuestSubID));
	}

	// For non-repeatable quests: only progress if we're not already past this step.
	// For repeatable quests: always allow re-progression on the same step.
	// Sentinel exception: when the current step is a FinishingStep, TryProgressQuest passes the
	// sentinel (QuestSubID == 0) as the "next" step to signal that the quest should complete.
	// Without this check, single-step quests whose only step has QuestSubID == 0 would hit
	// a false collision (sentinel.QuestSubID == 0 == ProgressID) and silently skip completion.
	// Log before the gate so we always know whether it passed.
	UDialogAndQuestPluginHelper::Log(FString::Printf(
		TEXT("ProgressQuest QID=%lld: NextSubID=%d ProgressID=%d Repeatable=%d FinishingStep=%d State=%d"),
		QuestMeta.QuestID, NextQuestStep.QuestSubID, QData.ProgressID, QData.Repeatable,
		QData.CurrentStep.FinishingStep, static_cast<int32>(QData.State)));

	const bool bNextIsSentinel = UQuestMainComponent::IsStepSentinel(NextQuestStep);
	if (bNextIsSentinel || NextQuestStep.QuestSubID != QData.ProgressID || QData.Repeatable)
	{
		QData.CurrentStep.Completed = true;

		if (!SkipReward && QData.CurrentStep.RewardAsset != nullptr)
			if (IQuestBearerInterface* SelfBearerInterface = Cast<IQuestBearerInterface>(GetOwner()))
				SelfBearerInterface->GrantReward(QData.CurrentStep.RewardAsset.Get());

		// Archive the completed step (for both repeatable and non-repeatable)
		QData.PreviousStep.Add(QData.CurrentStep);
		QData.ProgressID = NextQuestStep.QuestSubID;

		// Use the copy constructor so ALL FQuestStep fields (including StepType and
		// NextStepIDs) are preserved in the replicated FQuestProgressStep.  The
		// field-by-field approach previously used left StepType as Linear and
		// NextStepIDs empty, preventing the journal from detecting Branch steps.
		FQuestProgressStep NewStepProgress(NextQuestStep);
		NewStepProgress.Completed = false;

		// Build BranchAlternatives so the journal can render "OR" blocks when the player
		// faces a branching or parallel choice.  Cleared for non-dispatch steps.
		QData.BranchAlternatives.Empty();
		if (NewStepProgress.StepType == EQuestStepType::Branch ||
			NewStepProgress.StepType == EQuestStepType::Parallel)
		{
			for (const int32 BranchID : NewStepProgress.NextStepIDs)
			{
				for (const FQuestStep& BranchStep : QuestMeta.Steps)
				{
					if (BranchStep.QuestSubID == BranchID)
					{
						QData.BranchAlternatives.Add(FQuestProgressStep(BranchStep));
						break;
					}
				}
			}
		}

		// Completion is driven by the step the player JUST COMPLETED, now archived as
		// PreviousStep.Last(), NOT by the destination step (NextQuestStep).
		// The old check (NextQuestStep.FinishingStep) caused multi-step quests to become
		// Completed the instant the first turn-in placed the player on the finishing step —
		// making it terminal before the finishing step's own items were turned in, so the
		// finishing step's reward was never granted.
		if (QData.PreviousStep.Last().FinishingStep)
		{
			NewStepProgress.Completed = true;
			QData.State = EQuestState::Completed;
		}
		else
		{
			// If we had been in Achieved state (partial turn-in), go back to Accepted
			if (QData.State == EQuestState::Achieved)
				QData.State = EQuestState::Accepted;
		}

		QData.CurrentStep = MoveTemp(NewStepProgress);

		if (!bSilent)
			QuestUpdateDispatcher.Broadcast(QData.QuestID, QData.CurrentStep.QuestSubID, QData.State);
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
		NewQuestData.QuestDescription = QuestMeta.QuestDescription;
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
		// Quest already exists in the player's journal.
		FQuestProgressData& Existing = KnownQuestData[KnownQuestDataLUT[QuestMeta.QuestID]];

		// Transition to Accepted when:
		//   - Quest was only Mentioned or Briefed (formal accept)
		//   - Quest is Completed and marked Repeatable (repeat cycle reset)
		const bool bShouldReset = Existing.State == EQuestState::Mentioned  ||
		                          Existing.State == EQuestState::Briefed     ||
		                          (Existing.Repeatable && Existing.State == EQuestState::Completed);

		if (bShouldReset)
		{
			Existing.State = EQuestState::Accepted;
			Existing.ProgressID = 0;
			Existing.PreviousStep.Empty();
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

	const FQuestProgressData& QuestData = GetKnownQuest(QuestID);

	// When the player is sitting at a Branch or Parallel dispatch node, IsAtStep() also
	// returns true for destination step IDs.  In that case CurrentStep is the dispatch
	// node itself (no NecessaryItems), so look up the matching BranchAlternatives entry
	// to find the correct NecessaryItems / NecessaryCoins for this destination path.
	const FQuestProgressStep* StepToValidate = &QuestData.CurrentStep;
	if ((QuestData.CurrentStep.StepType == EQuestStepType::Branch ||
		 QuestData.CurrentStep.StepType == EQuestStepType::Parallel) &&
		QuestData.CurrentStep.QuestSubID != StepID)
	{
		for (const FQuestProgressStep& Alt : QuestData.BranchAlternatives)
		{
			if (Alt.QuestSubID == StepID)
			{
				StepToValidate = &Alt;
				break;
			}
		}
	}

	const bool IsExpectingSomething = !StepToValidate->NecessaryItems.IsEmpty() || StepToValidate->NecessaryCoins != 0.f;
	if (!IsExpectingSomething)
		return false;

	TArray<int32> NecessaryItems = StepToValidate->NecessaryItems;
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
	float NecessaryCoins = StepToValidate->NecessaryCoins - InputCoins;
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

	// Reject progression attempts on Mentioned/Briefed-state quests — they require a formal
	// AuthorityAddQuest (accept) before any step can be validated.
	if (IsQuestKnown(QuestID))
	{
		const EQuestState S = GetQuestState(QuestID);
		if (S == EQuestState::Mentioned || S == EQuestState::Briefed)
			return false;
	}

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

const FQuestProgressData* UQuestBearerComponent::GetKnownQuestSafe(int64 QuestID) const
{
	const int32* Index = KnownQuestDataLUT.Find(QuestID);
	if (!Index || *Index < 0 || *Index >= KnownQuestData.Num())
		return nullptr;
	return &KnownQuestData[*Index];
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
	{
		if (!IsQuestKnown(QuestID)) return true;
		const EQuestState S = GetKnownQuest(QuestID).State;
		return S == EQuestState::Mentioned || S == EQuestState::Briefed;
	}

	if (!IsQuestKnown(QuestID))
		return false;

	if (IsPastStep(QuestID, StepID))
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsBeforeStep(int64 QuestID, int32 StepID) const
{
	const FQuestProgressData* Quest = GetKnownQuestSafe(QuestID);
	return Quest && Quest->QuestID > 0 && Quest->ProgressID < StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsBeforeOrAtStep(int64 QuestID, int32 StepID) const
{
	const FQuestProgressData* Quest = GetKnownQuestSafe(QuestID);
	return Quest && Quest->QuestID > 0 && Quest->ProgressID <= StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsPastStep(int64 QuestID, int32 StepID) const
{
	const FQuestProgressData* Quest = GetKnownQuestSafe(QuestID);
	return Quest && Quest->ProgressID > StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsAtOrPastStep(int64 QuestID, int32 StepID) const
{
	const FQuestProgressData* Quest = GetKnownQuestSafe(QuestID);
	return Quest && Quest->ProgressID >= StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsAtStep(int64 QuestID, int32 StepID) const
{
	if (!IsQuestKnown(QuestID))
		return false;

	const FQuestProgressData& Quest = GetKnownQuest(QuestID);

	// Exact match: player is at this step.
	if (Quest.ProgressID == StepID)
		return true;

	// Multi-path aware: when the player is sitting at a Branch or Parallel dispatch node,
	// any of its destination step IDs are considered "reachable" for the purpose of
	// item-turn-in checks in HandlePlayerGive.  This allows each branch NPC to match its
	// own destination independently without the player being physically advanced first.
	if (Quest.CurrentStep.StepType == EQuestStepType::Branch ||
		Quest.CurrentStep.StepType == EQuestStepType::Parallel)
		return Quest.CurrentStep.NextStepIDs.Contains(StepID);

	return false;
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
	NewQuestData.QuestDescription = QuestData.QuestDescription;
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

void UQuestBearerComponent::BriefQuest(int64 QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (KnownQuestDataLUT.Contains(QuestID))
	{
		// Only upgrade Mentioned → Briefed; no-op for Briefed or any state beyond.
		FQuestProgressData& QData = KnownQuestData[KnownQuestDataLUT[QuestID]];
		if (QData.State != EQuestState::Mentioned)
			return;

		QData.State = EQuestState::Briefed;
		QuestUpdateDispatcher.Broadcast(QuestID, -1, EQuestState::Briefed);
		OnRep_KnownQuest();
		return;
	}

	// Unknown → Briefed (player spoke to giver without a prior rumor).
	IDialogGameModeInterface* GM = Cast<IDialogGameModeInterface>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GetMainQuestComponent())
		return;

	const FQuestMetaData& QuestData = GM->GetMainQuestComponent()->GetQuestData(QuestID);
	if (QuestData.QuestID == 0)
		return;

	FQuestProgressData NewQuestData;
	NewQuestData.QuestID              = QuestData.QuestID;
	NewQuestData.QuestTitle           = QuestData.QuestTitle;
	NewQuestData.QuestDescription     = QuestData.QuestDescription;
	NewQuestData.MentionedDescription = QuestData.MentionedDescription;
	NewQuestData.Repeatable           = QuestData.Repeatable;
	NewQuestData.State                = EQuestState::Briefed;
	NewQuestData.ProgressID           = -1;
	KnownQuestData.Add(MoveTemp(NewQuestData));

	RebuildQuestLUT();
	QuestUpdateDispatcher.Broadcast(QuestID, -1, EQuestState::Briefed);
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

	// Handle Briefed state — create a briefed entry, don't set up steps
	if (InitialState == EQuestState::Briefed)
	{
		BriefQuest(QuestID);
		return;
	}

	AuthorityAddQuest(QuestID);
	if (!KnownQuestDataLUT.Contains(QuestID))
		return;

	// Replay steps silently by walking the actual quest step graph rather than using a naive
	// counter loop.  A counter loop (old implementation) assumed QuestSubID values were
	// sequential 0-based integers, which is only true for auto-sequenced quests and breaks
	// completely for any quest with sparse or non-zero step IDs.
	//
	// Algorithm: starting from the initial state after AddQuest (ProgressID == 0,
	// CurrentStep == Steps[0]), follow FindNextStep on CurrentStep.QuestSubID each iteration
	// until ProgressID matches the persisted StepID (the s field from the backend).
	//
	// StepID == 0  →  player just accepted, no progressions needed (ProgressID already 0).
	// StepID  > 0  →  walk forward until ProgressID reaches StepID.
	if (StepID != 0)
	{
		// Safety cap: can't visit more unique steps than the quest defines.
		const int32 SafetyLimit = Meta.Steps.Num();
		int32 StepCount = 0;

		while (StepCount < SafetyLimit)
		{
			// Re-fetch after each ProgressQuest call since it may rebuild the LUT.
			const FQuestProgressData& Current = KnownQuestData[KnownQuestDataLUT[QuestID]];

			if (Current.ProgressID == StepID)
				break; // reached the target step

			if (Current.IsTerminal())
				break; // quest completed during replay (FinishingStep archived)

			// Navigate using CurrentStep.QuestSubID, not ProgressID.
			// After AddQuest, ProgressID==0 but CurrentStep.QuestSubID==Steps[0].QuestSubID,
			// so using CurrentStep ensures the graph walk starts correctly even for non-zero step IDs.
			const FQuestStep& NextStep = MQC->FindNextStep(Meta, Current.CurrentStep.QuestSubID);
			if (UQuestMainComponent::IsStepSentinel(NextStep))
			{
				UDialogAndQuestPluginHelper::Warning(FString::Printf(
					TEXT("AuthoritySetupQuestData: QID=%lld — reached end of step chain before StepID=%d (only advanced to ProgressID=%d). Quest asset may be missing steps."),
					QuestID, StepID, Current.ProgressID));
				break;
			}

			ProgressQuest(Meta, NextStep, /*SkipReward=*/true, /*bSilent=*/true);
			++StepCount;
		}

		// Log if we couldn't reach the target step — indicates a save/definition mismatch.
		const int32 FinalProgressID = KnownQuestData[KnownQuestDataLUT[QuestID]].ProgressID;
		if (FinalProgressID != StepID && !KnownQuestData[KnownQuestDataLUT[QuestID]].IsTerminal())
		{
			UDialogAndQuestPluginHelper::Warning(FString::Printf(
				TEXT("AuthoritySetupQuestData: QID=%lld — could not reach StepID=%d; ended at ProgressID=%d after %d iterations"),
				QuestID, StepID, FinalProgressID, StepCount));
		}
	}

	// Apply the requested initial state (Achieved, Completed, Botched, etc.)
	if (InitialState != EQuestState::Accepted)
	{
		FQuestProgressData& QData = KnownQuestData[KnownQuestDataLUT[QuestID]];
		QData.State = InitialState;
		// Propagate the corrected state to the owning client.
		OnRep_KnownQuest();
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
