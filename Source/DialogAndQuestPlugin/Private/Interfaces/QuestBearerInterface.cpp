// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/QuestBearerInterface.h"


// Add default functionality here for any IQuestBearerInterface functions that are not pure virtual.
void IQuestBearerInterface::TryProgressQuest(int64 QuestID, AActor* Validator)
{
	GetQuestBearerComponent()->TryProgressQuest(QuestID, Validator);
}

//----------------------------------------------------------------------------------------------------------------------

void IQuestBearerInterface::TryProgressAll(AActor* Validator)
{
	GetQuestBearerComponent()->TryProgressAll(Validator);
}

//----------------------------------------------------------------------------------------------------------------------

const FQuestProgressData& IQuestBearerInterface::GetKnownQuest(int64 QuestID) const
{
	return GetQuestBearerComponentConst()->GetKnownQuest(QuestID);
}

//----------------------------------------------------------------------------------------------------------------------

bool IQuestBearerInterface::IsQuestKnown(int64 QuestID) const
{
	return GetQuestBearerComponentConst()->IsQuestKnown(QuestID);
}

//----------------------------------------------------------------------------------------------------------------------

void IQuestBearerInterface::ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep)
{
	GetQuestBearerComponent()->ProgressQuest(QuestMeta, NextQuestStep);
}

//----------------------------------------------------------------------------------------------------------------------

void IQuestBearerInterface::AddQuest(const FQuestMetaData& QuestMeta)
{
	GetQuestBearerComponent()->AddQuest(QuestMeta);
}

//----------------------------------------------------------------------------------------------------------------------

void IQuestBearerInterface::GrantReward(TSubclassOf<UQuestRewardData> RewardData)
{
}

//----------------------------------------------------------------------------------------------------------------------

bool IQuestBearerInterface::CanValidate(int64 QuestID, int32 StepID) const
{
	return GetQuestBearerComponentConst()->CanValidate(QuestID, StepID);
}

//----------------------------------------------------------------------------------------------------------------------

bool IQuestBearerInterface::CanDisplay(int64 QuestID, int32 StepID, EQuestStepConditionType Condition) const
{
	return GetQuestBearerComponentConst()->CanDisplay(QuestID, StepID, Condition);
}
