// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/QuestMainComponent.h"

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

void UQuestMainComponent::AddQuest(const FQuestMetaData& QuestData)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	QuestList.Add(QuestData.QuestID, QuestData);
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

void UQuestMainComponent::TryProgressQuest(int64 QuestID, APlayerController* QuestBearer, AActor* Validator)
{
	IQuestBearerInterface* QuestBearerInterface = Cast<IQuestBearerInterface>(QuestBearer);
	if (!QuestBearerInterface)
	{
		UDialogAndQuestPluginHelper::Error("Tried to progress a quest for a non quest bearer");
		return;
	}

	IQuestGiverInterface* QuestGiverInterface = Cast<IQuestGiverInterface>(Validator);
	if (!QuestGiverInterface)
	{
		UDialogAndQuestPluginHelper::Error("Tried to progress a quest from a non quest giver");
		return;
	}

	const FQuestMetaData& CurrentQuest = GetQuestData(QuestID);

	int32 CurrentStepID = -1;
	if (QuestBearerInterface->IsQuestKnown(QuestID))
	{
		const FQuestProgressData& CurrentQuestProgress = QuestBearerInterface->GetKnownQuest(QuestID);
		CurrentStepID = CurrentQuestProgress.CurrentStep.QuestSubID;
	}

	if (QuestGiverInterface->GetQuestGiverComponent()->CanValidateQuestStep(
		QuestID, FindNextStepID(CurrentQuest, CurrentStepID)))
	{
		if (CurrentStepID == -1)
		{
			UDialogAndQuestPluginHelper::Log("Adding Quest");
			QuestBearerInterface->AddQuest(CurrentQuest);
		}
		else
		{
			UDialogAndQuestPluginHelper::Log("Progressing Quest");
			QuestBearerInterface->ProgressQuest(CurrentQuest, FindNextStep(CurrentQuest, CurrentStepID));
		}
	}
	else
	{
		UDialogAndQuestPluginHelper::Warning("Tried to validate an impossible quest state");
	}
}
