// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestData.h"
#include "QuestMainComponent.generated.h"


///@brief
/// This class is designed to contain all the needed quest, server side only and be component of the GameMode
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIALOGANDQUESTPLUGIN_API UQuestMainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQuestMainComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TMap<int64, FQuestMetaData> QuestList;

	uint32 FindNextStepID(const FQuestMetaData& QuestData, int32 CurrentStep);
public:

	const FQuestStep& FindNextStep(const FQuestMetaData& QuestData, int32 CurrentStep);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddQuest(const FQuestMetaData& QuestData);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddQuestFromDataTable(UDataTable* DataTable);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	const FQuestMetaData& GetQuestData(int64 QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ForceAddPlayerQuest(APlayerController* PlayerController, int64 QuestID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryProgressQuest(int64 QuestID, APlayerController* QuestBearer, AActor* Validator);
};
