// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/QuestMainComponent.h"

#include "Interfaces/DialogDisplayInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "Misc/DialogAndQuestPluginHelper.h"


// Sets default values for this component's properties
UQuestMainComponent::UQuestMainComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UQuestMainComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

uint32 UQuestMainComponent::FindNextStepID(const FQuestMetaData& QuestData, int32 CurrentStep)
{
	if (CurrentStep == -1)
		return 0;

	for (size_t StepID = 0; StepID < QuestData.Steps.Num() - 1; ++StepID)
	{
		if (QuestData.Steps[StepID].QuestSubID == CurrentStep)
		{
			return QuestData.Steps[StepID + 1].QuestSubID;
		}
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------

const FQuestStep& UQuestMainComponent::FindNextStep(const FQuestMetaData& QuestData, int32 CurrentStep)
{
	for (size_t StepID = 0; StepID < QuestData.Steps.Num() - 1; ++StepID)
	{
		if (QuestData.Steps[StepID].QuestSubID == CurrentStep)
		{
			return QuestData.Steps[StepID + 1];
		}
	}

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
	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const auto& Row : RowNames)
	{
		if (const FQuestMetaData* Item = DataTable->FindRow<FQuestMetaData>(Row, ""))
			AddQuest(*Item);
	}
}

//----------------------------------------------------------------------------------------------------------------------

const FQuestMetaData& UQuestMainComponent::GetQuestData(int64 QuestID) const
{
	return QuestList.FindChecked(QuestID);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestMainComponent::ForceAddPlayerQuest(APlayerController* PlayerController, int64 QuestID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (IQuestBearerInterface* QuestBearerInterface = Cast<IQuestBearerInterface>(PlayerController);
		QuestBearerInterface != nullptr)
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

	int32 CurrentStepID = -1;
	if (QuestBearerInterface->IsQuestKnown(QuestID))
	{
		const FQuestProgressData& CurrentQuestProgress = QuestBearerInterface->GetKnownQuest(QuestID);
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

		const FString& ProgressDialog = QuestBearerInterface->GetKnownQuest(CurrentQuest.QuestID).CurrentStep.
																	   ItemTurnInDialog;
		if (!ProgressDialog.IsEmpty())
		{
			IDialogDisplayInterface* DialogInterface = Cast<IDialogDisplayInterface>(QuestBearer);
			DialogInterface->ForceDisplayTextInDialog(ProgressDialog);
		}

		QuestBearerInterface->ProgressQuest(CurrentQuest, FindNextStep(CurrentQuest, CurrentStepID));

		return true;

	}

	FString WarningMessage = "Tried to validate an impossible quest state QID: " + FString::FormatAsNumber(CurrentQuest.QuestID) + " Current step : " +  FString::FormatAsNumber(CurrentStepID);
	UDialogAndQuestPluginHelper::Warning(WarningMessage);
	return false;

}
