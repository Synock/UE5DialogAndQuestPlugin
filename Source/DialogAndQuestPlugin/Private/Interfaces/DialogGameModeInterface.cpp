// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/DialogGameModeInterface.h"
<<<<<<< HEAD


// Add default functionality here for any IDialogGameModeInterface functions that are not pure virtual.
=======
#include "Misc/DialogAndQuestPluginHelper.h"


// Add default functionality here for any IDialogGameModeInterface functions that are not pure virtual.
void IDialogGameModeInterface::TryProgressQuest(int64 QuestID, APlayerController* QuestBearer, AActor* Validator)
{
	UQuestMainComponent* MasterQuestComponent = GetMainQuestComponent();
	if (!MasterQuestComponent)
	{
		UDialogAndQuestPluginHelper::Error("Invalid quest master component");
		return;
	}

	MasterQuestComponent->TryProgressQuest(QuestID, QuestBearer, Validator);
}
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
