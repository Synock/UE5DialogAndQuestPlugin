// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/DialogMainComponent.h"
#include "Components/QuestMainComponent.h"
#include "UObject/Interface.h"
#include "DialogGameModeInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UDialogGameModeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class DIALOGANDQUESTPLUGIN_API IDialogGameModeInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UDialogMainComponent* GetMainDialogComponent() = 0;

	virtual UQuestMainComponent* GetMainQuestComponent() = 0;

	virtual bool TryProgressQuest(int64 QuestID, APlayerController* QuestBearer, AActor* Validator);
};
