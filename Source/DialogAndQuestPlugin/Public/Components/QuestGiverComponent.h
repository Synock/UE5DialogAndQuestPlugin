// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestData.h"
#include "QuestGiverComponent.generated.h"


///@brief
/// This Component is not meant to be replicated but should only be defined on server
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIALOGANDQUESTPLUGIN_API UQuestGiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQuestGiverComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TMap<int64, FQuestValidatableSteps> ValidatableSteps;
public:

	UFUNCTION(BlueprintCallable)
	void AddValidatableSteps(int64 QuestID, TArray<int32> Steps);

	bool CanValidateQuestStep(int64 QuestID, int32 CurrentQuestStep);

	const TMap<int64, FQuestValidatableSteps>& GetValidatableQuestSteps() const {return ValidatableSteps;}
};
