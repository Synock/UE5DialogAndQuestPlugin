#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestData.h"
#include "QuestMainComponent.generated.h"

class UQuestAsset;
class IQuestGiverInterface;


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

	/// Returns the step to advance to after CurrentStep.
	/// When CurrentStep is a Branch and Validator is provided, picks the branch whose ID the
	/// validator's QuestGiverComponent has registered, enabling NPC-specific path selection.
	/// Without a Validator (or when no registered branch matches), falls back to NextStepIDs[0].
	/// Returns GQuestStepSentinel (QuestID==0, QuestSubID==0) when there is no next step.
	const FQuestStep& FindNextStep(const FQuestMetaData& QuestData, int32 CurrentStep,
	                               const IQuestGiverInterface* Validator = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddQuest(const FQuestMetaData& QuestData);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddQuestFromDataTable(UDataTable* DataTable);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddQuestFromAsset(UQuestAsset* QuestAsset);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	const FQuestMetaData& GetQuestData(int64 QuestID) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	TArray<FQuestMetaData> GetAllQuestData() const;

	bool TryGetQuestStep(const FQuestMetaData& QuestData, int32 StepID, FQuestStep& OutStep) const;

	bool TryBuildPathToStep(const FQuestMetaData& QuestData, int32 TargetStepID, TArray<FQuestStep>& OutPath) const;

	bool IsStepReachable(const FQuestMetaData& QuestData, int32 TargetStepID) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ForceAddPlayerQuest(APlayerController* PlayerController, int64 QuestID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryProgressQuest(int64 QuestID, APlayerController* QuestBearer, AActor* Validator);

	/// Returns the number of quest definitions currently registered.
	/// Used for startup log messages; also useful for editor debugging.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 GetRegisteredQuestCount() const { return QuestList.Num(); }

	/// Returns true if Step is the "no successor" sentinel returned by FindNextStep()
	/// when a step has no further steps in the quest chain.
	/// A sentinel step has QuestID == 0 and QuestSubID == 0 (default-constructed FQuestStep).
	/// Never pass a sentinel to ProgressQuest().
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
	static bool IsStepSentinel(const FQuestStep& Step) { return Step.QuestID == 0 && Step.QuestSubID == 0; }
};
