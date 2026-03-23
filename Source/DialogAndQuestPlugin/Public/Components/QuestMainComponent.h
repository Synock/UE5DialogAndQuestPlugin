#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestData.h"
#include "QuestMainComponent.generated.h"

class UQuestAsset;


///@brief
/// Server-side quest registry. Lives as a component on the GameMode (via IDialogGameModeInterface).
///
/// Holds the authoritative FQuestMetaData definitions for all quests in the zone. When a
/// player tries to progress a quest, TryProgressQuest() validates the request here:
///   1. Looks up the quest definition.
///   2. Checks the QuestGiverComponent on the NPC can validate the next step.
///   3. If valid, calls QuestBearerComponent::ProgressQuest() on the player.
///
/// Quest data is loaded at startup via AddQuestFromDataTable() or AddQuestFromAsset().
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
	void AddQuestFromAsset(UQuestAsset* QuestAsset);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	const FQuestMetaData& GetQuestData(int64 QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ForceAddPlayerQuest(APlayerController* PlayerController, int64 QuestID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryProgressQuest(int64 QuestID, APlayerController* QuestBearer, AActor* Validator);
};
