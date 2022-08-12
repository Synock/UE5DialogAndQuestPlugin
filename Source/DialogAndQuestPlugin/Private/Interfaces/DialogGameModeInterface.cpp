// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/DialogGameModeInterface.h"
#include "Misc/DialogAndQuestPluginHelper.h"


bool IDialogGameModeInterface::TryProgressQuest(int64 QuestID, APlayerController* QuestBearer, AActor* Validator)
{
	UQuestMainComponent* MasterQuestComponent = GetMainQuestComponent();
	if (!MasterQuestComponent)
	{
		UDialogAndQuestPluginHelper::Error("Invalid quest master component");
		return false;
	}

	return MasterQuestComponent->TryProgressQuest(QuestID, QuestBearer, Validator);
}
