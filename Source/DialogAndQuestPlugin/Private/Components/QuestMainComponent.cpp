#include "Components/QuestMainComponent.h"

#include "Quest/QuestAsset.h"
#include "Interfaces/DialogDisplayInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "Misc/DialogAndQuestPluginHelper.h"

// Returned by FindNextStep() when a step has no successor (last step or unknown step ID).
// Identified by QuestID == 0 && QuestSubID == 0.  Never pass to ProgressQuest().
static const FQuestStep GQuestStepSentinel;


UQuestMainComponent::UQuestMainComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestMainComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

// FindNextStep — single source of truth for step navigation.
// When CurrentStep is a Branch step and Validator is provided, the branch whose ID is
// registered on the validator's QuestGiverComponent is selected.  This lets each NPC
// steer the player down its own story path without the system hard-wiring NextStepIDs[0].
// Without a validator (restore walk, journal preview) the first branch is the fallback.
// Returns GQuestStepSentinel (QuestID==0, QuestSubID==0) when there is no successor.
// Callers must check IsStepSentinel() before passing the result to ProgressQuest().
const FQuestStep& UQuestMainComponent::FindNextStep(const FQuestMetaData& QuestData, int32 CurrentStep,
                                                    const IQuestGiverInterface* Validator)
{
	// Special case: quest just accepted (ProgressID == -1), return first step.
	if (CurrentStep == -1 && !QuestData.Steps.IsEmpty())
		return QuestData.Steps[0];

	for (int32 i = 0; i < QuestData.Steps.Num(); ++i)
	{
		const FQuestStep& Step = QuestData.Steps[i];
		if (Step.QuestSubID != CurrentStep)
			continue;

		// Non-linear step: use NextStepIDs to resolve the destination.
		if (Step.StepType != EQuestStepType::Linear && !Step.NextStepIDs.IsEmpty())
		{
			// Branch: when a validator is provided, pick the first branch ID that the
			// validator's QuestGiverComponent has registered.  Return sentinel if the
			// validator owns none of the destinations — this signals TryProgressQuest that
			// this NPC has no authority over any branch path from this node.
			// Without a validator (restore graph-walk, journal BranchAlternatives preview)
			// fall back to NextStepIDs[0] so those code paths continue to work.
			if (Validator)
			{
				const UQuestGiverComponent* GiverComp = Validator->GetQuestGiverComponent();
				if (GiverComp)
				{
					for (const int32 BranchID : Step.NextStepIDs)
					{
						if (GiverComp->CanValidateQuestStep(QuestData.QuestID, BranchID))
						{
							for (const FQuestStep& Candidate : QuestData.Steps)
								if (Candidate.QuestSubID == BranchID) return Candidate;
						}
					}
				}
				// Validator provided but no owned destination found — reject.
				return GQuestStepSentinel;
			}

			// No validator: use first NextStepID as fallback (restore walk / preview).
			const int32 FallbackID = Step.NextStepIDs[0];
			for (const FQuestStep& Candidate : QuestData.Steps)
				if (Candidate.QuestSubID == FallbackID) return Candidate;

			// Target ID not found — quest data is malformed; fall through to sentinel.
		}

		// Linear: if this step is a FinishingStep, no successor exists — the quest ends here.
		// Without this guard, steps that are FinishingStep=true but not the last element in the
		// array (e.g. branch destination steps sitting alongside other path steps) would
		// incorrectly return the next sibling as the "next step", causing the wrong step to be
		// passed to ProgressQuest and breaking quest completion.
		if (Step.FinishingStep)
			return GQuestStepSentinel;

		// Linear: advance to the next element in the array.
		if (i + 1 < QuestData.Steps.Num())
			return QuestData.Steps[i + 1];

		// This is the last step — return sentinel so callers know there is no next step.
		return GQuestStepSentinel;
	}

	// Step ID not found in the quest definition.
	return GQuestStepSentinel;
}

//----------------------------------------------------------------------------------------------------------------------

// Delegates to FindNextStep to avoid duplicated navigation logic.
uint32 UQuestMainComponent::FindNextStepID(const FQuestMetaData& QuestData, int32 CurrentStep)
{
	return static_cast<uint32>(FindNextStep(QuestData, CurrentStep).QuestSubID);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestMainComponent::AddQuest(const FQuestMetaData& QuestData)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	QuestList.Add(QuestData.QuestID, QuestData);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestMainComponent::AddQuestFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
		return;

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const auto& Row : RowNames)
	{
		if (const FQuestMetaData* Item = DataTable->FindRow<FQuestMetaData>(Row, ""))
			AddQuest(*Item);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestMainComponent::AddQuestFromAsset(UQuestAsset* QuestAsset)
{
	if (!QuestAsset)
		return;

	// Idempotent — safe to call multiple times for the same asset/class (e.g., during
	// auto-scan at StartPlay and again in SpawnNPC for runtime-spawned actors).
	if (QuestList.Contains(QuestAsset->QuestID))
		return;

	AddQuest(QuestAsset->ToQuestMetaData());
}

//----------------------------------------------------------------------------------------------------------------------

const FQuestMetaData& UQuestMainComponent::GetQuestData(int64 QuestID) const
{
	if (const FQuestMetaData* Found = QuestList.Find(QuestID))
		return *Found;

	UDialogAndQuestPluginHelper::Warning(FString::Printf(TEXT("Quest ID %lld not found in quest registry"), QuestID));

	static const FQuestMetaData EmptyQuest;
	return EmptyQuest;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestMainComponent::ForceAddPlayerQuest(APlayerController* PlayerController, int64 QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (IQuestBearerInterface* QuestBearerInterface = Cast<IQuestBearerInterface>(PlayerController))
	{
		QuestBearerInterface->GetQuestBearerComponent()->AuthorityAddQuest(QuestID);
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestMainComponent::TryProgressQuest(int64 QuestID, APlayerController* QuestBearer, AActor* Validator)
{
	IQuestBearerInterface* QuestBearerInterface = Cast<IQuestBearerInterface>(QuestBearer);
	if (!QuestBearerInterface)
	{
		UDialogAndQuestPluginHelper::Error("Tried to progress a quest for a non quest bearer");
		return false;
	}

	const IQuestGiverInterface* QuestGiverInterface = Cast<IQuestGiverInterface>(Validator);
	if (!QuestGiverInterface)
	{
		UDialogAndQuestPluginHelper::Error("Tried to progress a quest from a non quest giver");
		return false;
	}

	const FQuestMetaData& CurrentQuest = GetQuestData(QuestID);
	if (CurrentQuest.QuestID == 0)
		return false;

	int32 CurrentStepID = -1;
	if (QuestBearerInterface->IsQuestKnown(QuestID))
	{
		const FQuestProgressData& CurrentQuestProgress = QuestBearerInterface->GetKnownQuest(QuestID);

		// Don't progress terminal quests
		if (CurrentQuestProgress.IsTerminal())
			return false;

		// Block progression attempts on Mentioned-state quests via a crafted RPC.
		// Mentioned quests must go through AuthorityAddQuest (formal accept) first.
		if (CurrentQuestProgress.State == EQuestState::Mentioned)
		{
			UDialogAndQuestPluginHelper::Warning(FString::Printf(
				TEXT("TryProgressQuest: QID=%lld is in Mentioned state — formal accept required before progression"),
				QuestID));
			return false;
		}

		CurrentStepID = CurrentQuestProgress.CurrentStep.QuestSubID;
	}

	// Resolve the next step now, passing the validator so Branch steps pick the
	// correct story path for this NPC.  FindNextStep returns GQuestStepSentinel when
	// there is no successor (finishing step, malformed data, or NPC owns no branch dest).
	const FQuestStep& NextStep = FindNextStep(CurrentQuest, CurrentStepID, QuestGiverInterface);
	const int32 NextQuestStepID = NextStep.QuestSubID;

	// Determine which step ID to present to the QuestGiverComponent's validatable list:
	//
	//  • Initial accept (CurrentStepID == -1): validate the first step ID.
	//  • Branch current step: validate via the DESTINATION step the NPC owns.
	//    FindNextStep already did the ownership lookup; if it returned sentinel the NPC
	//    owns no destination from this branch → reject immediately.
	//    If it returned a real step, use that step's ID as the validator key.
	//  • Linear current step: validate the step being COMPLETED (CurrentStepID).
	//    This prevents the finishing-step check from requiring step 0 registered.
	// Determine whether the player is currently sitting at a multi-path dispatch node
	// (Branch or Parallel).  This flag drives: destination-step ValidatorID selection,
	// sentinel rejection, ItemTurnInDialog source, and the finishing-step auto-advance.
	const bool bCurrentIsMultiPath = (CurrentStepID != -1) &&
		QuestBearerInterface->IsQuestKnown(QuestID) &&
		(QuestBearerInterface->GetKnownQuest(QuestID).CurrentStep.StepType == EQuestStepType::Branch ||
		 QuestBearerInterface->GetKnownQuest(QuestID).CurrentStep.StepType == EQuestStepType::Parallel);

	if (bCurrentIsMultiPath && IsStepSentinel(NextStep))
	{
		UDialogAndQuestPluginHelper::Warning(FString::Printf(
			TEXT("TryProgressQuest: QID=%lld — NPC has no registered destination for Branch step %d, rejecting"),
			QuestID, CurrentStepID));
		return false;
	}

	const int32 ValidatorStepID = (CurrentStepID == -1) ? NextQuestStepID
	                            : bCurrentIsMultiPath    ? NextQuestStepID   // destination the NPC owns
	                            :                          CurrentStepID;     // step being completed

	const bool bCanValidate = QuestGiverInterface->GetQuestGiverComponent()->CanValidateQuestStep(QuestID, ValidatorStepID);
	UDialogAndQuestPluginHelper::Log(FString::Printf(
		TEXT("TryProgressQuest QID=%lld CurrentStep=%d NextStep=%d ValidatorStep=%d CanValidate=%d"),
		QuestID, CurrentStepID, NextQuestStepID, ValidatorStepID, bCanValidate));

	if (bCanValidate)
	{
		if (CurrentStepID == -1)
		{
			// Accept path: quest is unknown — sentinel means no steps defined (malformed quest).
			if (IsStepSentinel(NextStep))
			{
				UDialogAndQuestPluginHelper::Warning(FString::Printf(
					TEXT("TryProgressQuest: QID=%lld has no steps — cannot accept"), QuestID));
				return false;
			}

			UDialogAndQuestPluginHelper::Log("Adding Quest id : " + FString::FormatAsNumber(CurrentQuest.QuestID));
			QuestBearerInterface->AddQuest(CurrentQuest);
			return true;
		}

		UDialogAndQuestPluginHelper::Log("Progressing Quest id : " + FString::FormatAsNumber(CurrentQuest.QuestID) + " step : " + FString::FormatAsNumber(CurrentStepID));

		// For Branch/Parallel dispatch nodes, ItemTurnInDialog is defined on the destination
		// step (NextStep), not on the dispatch node itself (CurrentStep).  For all other step
		// types the dialog lives on the step being completed (CurrentStep).
		const FText ProgressDialog = bCurrentIsMultiPath
			? NextStep.ItemTurnInDialog
			: QuestBearerInterface->GetKnownQuest(CurrentQuest.QuestID).CurrentStep.ItemTurnInDialog;
		if (!ProgressDialog.IsEmpty())
		{
			if (IDialogDisplayInterface* DialogInterface = Cast<IDialogDisplayInterface>(QuestBearer))
				DialogInterface->ForceDisplayTextInDialog(ProgressDialog.ToString());
		}

		// NextStep may be the sentinel when CurrentStepID is the FinishingStep.
		// ProgressQuest handles this correctly by checking PreviousStep.Last().FinishingStep.
		QuestBearerInterface->ProgressQuest(CurrentQuest, NextStep);

		// Multi-path → FinishingStep auto-completion:
		// When resolving a Branch/Parallel dispatch, CanValidateStepWithItems already consumed
		// the required items against the destination step's NecessaryItems list.  The destination
		// step is now CurrentStep but the player has no items left to trigger a second turn-in.
		// Advance one more time (to sentinel) so the finishing step is archived, its reward is
		// granted, and the quest becomes Completed — all in the same player action.
		if (bCurrentIsMultiPath && !IsStepSentinel(NextStep) && NextStep.FinishingStep)
		{
			// FindNextStep(quest, FinishingStepID) returns GQuestStepSentinel by design
			// (see the FinishingStep guard in FindNextStep above).
			const FQuestStep& FinishSentinel = FindNextStep(CurrentQuest, NextStep.QuestSubID, QuestGiverInterface);
			QuestBearerInterface->ProgressQuest(CurrentQuest, FinishSentinel);
		}

		return true;
	}

	FString WarningMessage = FString::Printf(
		TEXT("TryProgressQuest: cannot validate QID=%lld CurrentStep=%d ValidatorStep=%d — NPC lacks this step in its validatable list"),
		QuestID, CurrentStepID, ValidatorStepID);
	UDialogAndQuestPluginHelper::Warning(WarningMessage);
	return false;

}
