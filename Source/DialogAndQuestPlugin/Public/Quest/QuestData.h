// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"
#include "QuestData.generated.h"

UENUM(BlueprintType)
enum struct EQuestStepConditionType: uint8
{
	Equal,
	Lesser,
	LesserEqual,
	Greater,
	GreaterEqual
};

UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class UQuestRewardData : public UObject
{
	GENERATED_BODY()
};

///@brief Represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang
USTRUCT(BlueprintType)
struct FQuestStep : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 QuestSubID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString StepTitle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString StepDescription;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool FinishingStep = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UQuestRewardData> RewardClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int32> NecessaryItems;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float NecessaryCoins = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString ItemTurnInDialog;
};

///@brief Represent the complete quest info
USTRUCT(BlueprintType)
struct FQuestMetaData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString QuestTitle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Repeatable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FQuestStep> Steps;
};

///@brief This is a list of objectives that can be validated by a quest giver
USTRUCT(BlueprintType)
struct FQuestValidatableSteps : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int32> Steps;
};

///@brief Represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang and its completion state
USTRUCT(BlueprintType)
struct FQuestProgressStep : public FQuestStep
{
	GENERATED_BODY()
	FQuestProgressStep() = default;

	explicit FQuestProgressStep(const FQuestStep& Step);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
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
