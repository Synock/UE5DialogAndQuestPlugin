// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/QuestBearerComponent.h"
#include "UObject/Interface.h"
#include "QuestBearerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UQuestBearerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class DIALOGANDQUESTPLUGIN_API IQuestBearerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UQuestBearerComponent* GetQuestBearerComponent() = 0;

	virtual const UQuestBearerComponent* GetQuestBearerComponentConst() const = 0;

	virtual bool CanValidate(int64 QuestID, int32 StepID) const;

	virtual bool CanDisplay(int64 QuestID, int32 StepID, EQuestStepConditionType Condition = EQuestStepConditionType::Equal) const;

	//this is called from client
	virtual void TryProgressQuest(int64 QuestID, AActor* Validator);

	//this is called from client
	virtual void TryProgressAll(AActor* Validator);

	virtual const FQuestProgressData& GetKnownQuest(int64 QuestID) const;

	virtual bool IsQuestKnown(int64 QuestID) const;

	//this is called from Server
	virtual void ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep);

	//this is called from Server
	virtual void AddQuest(const FQuestMetaData& QuestMeta);

	virtual void GrantReward(TSubclassOf<UQuestRewardData> RewardData);
};
