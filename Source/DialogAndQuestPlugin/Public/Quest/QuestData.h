// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestData.generated.h"

<<<<<<< HEAD
///represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang
=======
///@brief Represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
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
<<<<<<< HEAD
};

/// This is a complete quest info
=======

	UPROPERTY(BlueprintReadOnly)
	bool FinishingStep = false;
};

///@brief Represent the complete quest info
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
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
<<<<<<< HEAD

};

///This is a list of objectives that can be validated
USTRUCT(BlueprintType)
struct FQuestValidableSteps
=======
};

///@brief This is a list of objectives that can be validated by a quest giver
USTRUCT(BlueprintType)
struct FQuestValidatableSteps
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> Steps;
};

<<<<<<< HEAD
///represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang and its completion state
=======
///@brief Represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang and its completion state
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
USTRUCT(BlueprintType)
struct FQuestProgressStep : public FQuestStep
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool Completed = false;
};

<<<<<<< HEAD
///This is a quest from the player perspective
=======
///@brief This is a quest from the player perspective with information on completion
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
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


<<<<<<< HEAD
///Global quest Data
=======
///@brief Global quest data
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
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

<<<<<<< HEAD
///quest Step entry
=======
///@brief Quest step entry
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
UCLASS(BlueprintType)
class DIALOGANDQUESTPLUGIN_API UQuestEntryData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FQuestProgressStep Data;

	UPROPERTY(BlueprintReadWrite)
	UQuestJournalWindow* Parent = nullptr;
<<<<<<< HEAD
};
=======
};
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
