// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/QuestMainComponent.h"

#include "Interfaces/QuestBearerInterface.h"
<<<<<<< HEAD
=======
#include "Interfaces/QuestGiverInterface.h"
#include "Misc/DialogAndQuestPluginHelper.h"
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84


// Sets default values for this component's properties
UQuestMainComponent::UQuestMainComponent()
{
<<<<<<< HEAD
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
=======
	PrimaryComponentTick.bCanEverTick = false;
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UQuestMainComponent::BeginPlay()
{
	Super::BeginPlay();
<<<<<<< HEAD

	// ...
=======
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
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

<<<<<<< HEAD
=======
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

>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
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
<<<<<<< HEAD
=======

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
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
