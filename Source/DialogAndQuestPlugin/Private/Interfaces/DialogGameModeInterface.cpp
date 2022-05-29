// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/DialogGameModeInterface.h"
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
