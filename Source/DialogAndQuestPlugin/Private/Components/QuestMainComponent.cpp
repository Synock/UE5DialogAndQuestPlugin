
#include "Components/QuestMainComponent.h"

#include "Quest/QuestAsset.h"
#include "Interfaces/DialogDisplayInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "Misc/DialogAndQuestPluginHelper.h"


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

uint32 UQuestMainComponent::FindNextStepID(const FQuestMetaData& QuestData, int32 CurrentStep)
{
	// Special case: quest just accepted (CurrentStep == -1), return first step
	if (CurrentStep == -1 && !QuestData.Steps.IsEmpty())
		return QuestData.Steps[0].QuestSubID;

	for (int32 i = 0; i < QuestData.Steps.Num(); ++i)
	{
		const FQuestStep& Step = QuestData.Steps[i];
		if (Step.QuestSubID != CurrentStep)
			continue;

		// Non-linear step with explicit next IDs — use the first one.
		// Game code can override which branch to take by calling FindNextStepID with a specific
		// NextStepID directly; this default picks branch 0.
		if (Step.StepType != EQuestStepType::Linear && !Step.NextStepIDs.IsEmpty())
			return static_cast<uint32>(Step.NextStepIDs[0]);

		// Linear: next element in the array
		if (i + 1 < QuestData.Steps.Num())
			return QuestData.Steps[i + 1].QuestSubID;

		// Already on the last step
		return 0;
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------

const FQuestStep& UQuestMainComponent::FindNextStep(const FQuestMetaData& QuestData, int32 CurrentStep)
{
	// Special case: quest just accepted, return first step
	if (CurrentStep == -1 && !QuestData.Steps.IsEmpty())
		return QuestData.Steps[0];

	for (int32 i = 0; i < QuestData.Steps.Num(); ++i)
	{
		const FQuestStep& Step = QuestData.Steps[i];
		if (Step.QuestSubID != CurrentStep)
			continue;

		// Non-linear: follow explicit NextStepIDs, find the step by ID
		if (Step.StepType != EQuestStepType::Linear && !Step.NextStepIDs.IsEmpty())
		{
			const int32 TargetID = Step.NextStepIDs[0];
			for (const FQuestStep& Candidate : QuestData.Steps)
			{
				if (Candidate.QuestSubID == TargetID)
					return Candidate;
			}
		}

		// Linear: next element in the array
		if (i + 1 < QuestData.Steps.Num())
			return QuestData.Steps[i + 1];

		break;
	}

	// Fallback to first step (should not be reached in well-formed quest data)
	return QuestData.Steps[0];
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

		CurrentStepID = CurrentQuestProgress.CurrentStep.QuestSubID;
	}

	const int32 NextQuestStep = FindNextStepID(CurrentQuest, CurrentStepID);
	if (QuestGiverInterface->GetQuestGiverComponent()->CanValidateQuestStep(QuestID, NextQuestStep))
	{
		if (CurrentStepID == -1)
		{
			UDialogAndQuestPluginHelper::Log("Adding Quest id : " + FString::FormatAsNumber(CurrentQuest.QuestID));
			QuestBearerInterface->AddQuest(CurrentQuest);
			return true;
		}

		UDialogAndQuestPluginHelper::Log("Progressing Quest id : " + FString::FormatAsNumber(CurrentQuest.QuestID) + " step : " + FString::FormatAsNumber(CurrentStepID));

		const FText& ProgressDialog = QuestBearerInterface->GetKnownQuest(CurrentQuest.QuestID).CurrentStep.ItemTurnInDialog;
		if (!ProgressDialog.IsEmpty())
		{
			if (IDialogDisplayInterface* DialogInterface = Cast<IDialogDisplayInterface>(QuestBearer))
				DialogInterface->ForceDisplayTextInDialog(ProgressDialog.ToString());
		}

		QuestBearerInterface->ProgressQuest(CurrentQuest, FindNextStep(CurrentQuest, CurrentStepID));
		return true;
	}

	FString WarningMessage = "Tried to validate an impossible quest state QID: " + FString::FormatAsNumber(CurrentQuest.QuestID) + " Current step : " + FString::FormatAsNumber(CurrentStepID);
	UDialogAndQuestPluginHelper::Warning(WarningMessage);
	return false;

}
