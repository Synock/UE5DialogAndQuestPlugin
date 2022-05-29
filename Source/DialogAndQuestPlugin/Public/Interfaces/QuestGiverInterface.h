// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/QuestGiverComponent.h"
#include "UObject/Interface.h"
#include "QuestGiverInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UQuestGiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class DIALOGANDQUESTPLUGIN_API IQuestGiverInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UQuestGiverComponent* GetQuestGiverComponent() const = 0;

	bool CanValidateQuestStep(int64 QuestID, int32 CurrentQuestStep);
};
