// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Quest/QuestData.h"
#include "DialogData.generated.h"

///@brief This represent a condition for a dialog to appear.
/// Ideally you should update this to include your own data
USTRUCT(BlueprintType)
struct DIALOGANDQUESTPLUGIN_API FDialogTopicCondition  : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int64 QuestId = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MinimumStepID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EQuestStepConditionType StepCondition = EQuestStepConditionType::Equal;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinimumRelation = 0.375f;
	
	bool VerifyCondition(const AActor* DialogActor, const APlayerController* Controller) const;
};

///@brief This struct represent a dialog topic and its potential quest relation
USTRUCT(BlueprintType)
struct FDialogTopicStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int64 Id = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Topic;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDialogTopicCondition TopicCondition;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString TopicText;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FQuestValidatableSteps QuestRelation;
};

///@brief this is a topic bundle, composed of several possible dialog topics
USTRUCT(BlueprintType)
struct FDialogTopicBundleStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int64 Id = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int64> TopicList;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MetaName;
};

///@brief This is a topic meta bundle, composed of several topic bundle
USTRUCT(BlueprintType)
struct FDialogTopicMetaBundleStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int64 Id = 0;

	///this is the greeting dialog when player relation is > MinimumRelation
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString GoodGreetingDialog = "Greetings";

	///this is the greeting dialog when player relation is < MinimumRelation
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString BadGreetingDialog = "I don't wish to speak to your kind. Now get lost!";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinimumRelation = 0.375;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int64> TopicBundleList;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MetaName;
};