// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestData.generated.h"

///@brief Represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang
USTRUCT(BlueprintType)
struct FQuestStep
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 QuestSubID = 0;

	UPROPERTY(BlueprintReadOnly)
	FString StepTitle;

	UPROPERTY(BlueprintReadOnly)
	FString StepDescription;

	UPROPERTY(BlueprintReadOnly)
	bool FinishingStep = false;
};

///@brief Represent the complete quest info
USTRUCT(BlueprintType)
struct FQuestMetaData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadOnly)
	FString QuestTitle;

	UPROPERTY(BlueprintReadOnly)
	bool Repeatable = false;

	UPROPERTY(BlueprintReadOnly)
	TArray<FQuestStep> Steps;
};

///@brief This is a list of objectives that can be validated by a quest giver
USTRUCT(BlueprintType)
struct FQuestValidatableSteps
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> Steps;
};

///@brief Represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang and its completion state
USTRUCT(BlueprintType)
struct FQuestProgressStep : public FQuestStep
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool Completed = false;
};

///@brief This is a quest from the player perspective with information on completion
USTRUCT(BlueprintType)
struct FQuestProgressData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ProgressID = 0;

	UPROPERTY(BlueprintReadOnly)
	bool Finished = false;

	UPROPERTY(BlueprintReadOnly)
	bool Repeatable = false;

	UPROPERTY(BlueprintReadOnly)
	FQuestProgressStep CurrentStep;

	UPROPERTY(BlueprintReadOnly)
	FString QuestTitle;

	///steps already done by the player
	UPROPERTY(BlueprintReadOnly)
	TArray<FQuestProgressStep> PreviousStep;
};


///@brief Global quest data
UCLASS(BlueprintType)
class DIALOGANDQUESTPLUGIN_API UQuestJournalTitleData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FQuestProgressData Data;

	UPROPERTY(BlueprintReadWrite)
	class UQuestJournalWindow* Parent = nullptr;
};

///@brief Quest step entry
UCLASS(BlueprintType)
class DIALOGANDQUESTPLUGIN_API UQuestEntryData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FQuestProgressStep Data;

	UPROPERTY(BlueprintReadWrite)
	UQuestJournalWindow* Parent = nullptr;
};
