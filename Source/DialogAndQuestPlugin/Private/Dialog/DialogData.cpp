// Copyright 2022 Maximilien (Synock) Guislain


#include "Dialog/DialogData.h"

#include "Interfaces/DialogInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"

bool FDialogTopicCondition::VerifyCondition(const AActor* DialogActor, const APlayerController* Controller) const
{
	bool StandardReturn = false;
	if (const IDialogInterface* DialogInterfaceActor = Cast<IDialogInterface>(DialogActor); DialogInterfaceActor)
		StandardReturn = DialogInterfaceActor->GetRelation(Controller->GetPawn()) >= MinimumRelation;

	if (QuestId != 0)
	{
		if (const IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(Controller))
		{
			if (const IQuestGiverInterface* QuestActor = Cast<IQuestGiverInterface>(DialogActor))
			{
				const bool QuestCondition = QuestBearer->CanDisplay(QuestId, MinimumStepID, StepCondition);

				return QuestCondition && StandardReturn;
			}
		}

		return false;
	}

	return StandardReturn;
}
