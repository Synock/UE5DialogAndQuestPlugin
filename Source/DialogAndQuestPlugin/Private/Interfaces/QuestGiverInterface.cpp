// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/QuestGiverInterface.h"


// Add default functionality here for any IQuestGiverInterface functions that are not pure virtual.
bool IQuestGiverInterface::CanValidateQuestStep(int64 QuestID, int32 CurrentQuestStep)
{
	return GetQuestGiverComponent()->CanValidateQuestStep(QuestID, CurrentQuestStep);
}
