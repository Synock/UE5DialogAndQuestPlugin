
#include "Interfaces/QuestGiverInterface.h"


// Add default functionality here for any IQuestGiverInterface functions that are not pure virtual.
bool IQuestGiverInterface::CanValidateQuestStep(int64 QuestID, int32 CurrentQuestStep) const
{
	return GetQuestGiverComponent()->CanValidateQuestStep(QuestID, CurrentQuestStep);
}
