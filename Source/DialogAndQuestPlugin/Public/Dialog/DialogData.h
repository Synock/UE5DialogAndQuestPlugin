// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Quest/QuestData.h"
#include "UObject/Object.h"
#include "DialogData.generated.h"

<<<<<<< HEAD

USTRUCT(BlueprintType)
struct FDialogTopicCondition
=======
///@brief This represent a condition for a dialog to appear.
/// Ideally you should update this to include your own data
USTRUCT(BlueprintType)
struct DIALOGANDQUESTPLUGIN_API FDialogTopicCondition
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
<<<<<<< HEAD
	float MinimumRelation = 0.375f;

	bool VerifyCondition(const AActor* DialogActor) const;
};

=======
	int64 QuestId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 MinimumStepID = 0;

	UPROPERTY(BlueprintReadWrite)
	float MinimumRelation = 0.375f;

	bool VerifyCondition(const AActor* DialogActor, const APlayerController* Controller) const;
};

///@brief This struct represent a dialog topic and its potential quest relation
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
USTRUCT(BlueprintType)
struct FDialogTopicStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int64 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Topic;

	UPROPERTY(BlueprintReadWrite)
	FDialogTopicCondition TopicCondition;

	UPROPERTY(BlueprintReadWrite)
	FString TopicText;

	UPROPERTY(BlueprintReadWrite)
<<<<<<< HEAD
	FQuestValidableSteps QuestRelation;
};

=======
	FQuestValidatableSteps QuestRelation;
};

///@brief this is a topic bundle, composed of several possible dialog topics
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
USTRUCT(BlueprintType)
struct FDialogTopicBundleStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int64 Id = 0;

	TArray<int64> TopicList;

	UPROPERTY(BlueprintReadWrite)
	FString MetaName;
};

<<<<<<< HEAD
=======
///@brief This is a topic meta bundle, composed of several topic bundle
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
USTRUCT(BlueprintType)
struct FDialogTopicMetaBundleStruct
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int64 Id = 0;

<<<<<<< HEAD
	UPROPERTY(BlueprintReadWrite)
	FString GoodGreetingDialog = "Greetings";

=======
	///this is the greeting dialog when player relation is > MinimumRelation
	UPROPERTY(BlueprintReadWrite)
	FString GoodGreetingDialog = "Greetings";

	///this is the greeting dialog when player relation is < MinimumRelation
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	UPROPERTY(BlueprintReadWrite)
	FString BadGreetingDialog = "I don't wish to speak to your kind. Now get lost!";

	UPROPERTY(BlueprintReadWrite)
	float MinimumRelation = 0.375;

	TArray<int64> TopicBundleList;

	UPROPERTY(BlueprintReadWrite)
	FString MetaName;
<<<<<<< HEAD
};

/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogData : public UObject
{
	GENERATED_BODY()
};
=======
};
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
