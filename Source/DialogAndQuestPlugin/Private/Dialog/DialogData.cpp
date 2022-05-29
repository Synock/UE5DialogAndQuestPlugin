// Copyright 2022 Maximilien (Synock) Guislain


#include "Dialog/DialogData.h"

#include "Interfaces/DialogInterface.h"
<<<<<<< HEAD

bool FDialogTopicCondition::VerifyCondition(const AActor* DialogActor) const
{
	if (const IDialogInterface* DialogInterfaceActor = Cast<IDialogInterface>(DialogActor))
		return DialogInterfaceActor->GetRelation() >= MinimumRelation;

	return false;
=======
#include "Interfaces/QuestBearerInterface.h"
#include "Interfaces/QuestGiverInterface.h"

bool FDialogTopicCondition::VerifyCondition(const AActor* DialogActor, const APlayerController* Controller) const
{
	bool StandardReturn = false;
	if (const IDialogInterface* DialogInterfaceActor = Cast<IDialogInterface>(DialogActor); DialogInterfaceActor)
		StandardReturn = DialogInterfaceActor->GetRelation() >= MinimumRelation;

	if (QuestId != 0)
	{
		if (const IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(Controller); QuestBearer)
		{
			if (const IQuestGiverInterface* QuestActor = Cast<IQuestGiverInterface>(DialogActor); QuestActor)
			{
				const bool QuestCondition = QuestBearer->CanDisplay(QuestId, MinimumStepID);

				return QuestCondition && StandardReturn;
			}
		}

		return false;
	}

	return StandardReturn;
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}
