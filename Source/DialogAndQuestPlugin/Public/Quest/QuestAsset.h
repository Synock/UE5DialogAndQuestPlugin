#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quest/QuestData.h"
#include "QuestAsset.generated.h"

/**
 * Unified quest asset that holds all quest metadata, steps, rewards, and botch conditions
 * in a single versionable asset. NPCs and systems reference this via TObjectPtr<UQuestAsset>.
 */
UCLASS(BlueprintType)
class DIALOGANDQUESTPLUGIN_API UQuestAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/// Unique quest identifier — must match across backend persistence.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int64 QuestID = 0;

	/// Display title in the quest journal.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText QuestTitle;

	/// Full description shown once the quest is accepted.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText QuestDescription;

	/// Rumor text for the Mentioned state — shown before the quest is formally accepted.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText MentionedDescription;

	/// Whether this quest can be repeated after completion.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool Repeatable = false;

	/// Ordered quest steps. Linear by default; use NextStepIDs on individual steps for branching.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Steps")
	TArray<FQuestStep> Steps;

	/// Conditions that irrecoverably botch this quest.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Botch")
	TArray<FQuestBotchCondition> BotchConditions;

	/**
	 * Item IDs that must stay in the player's possession (inventory or bank) while this
	 * quest is active. If ANY of these items is dropped, sold, or destroyed,
	 * ANeverQuestGameMode::HandleQuestBotchOnItemLoss() calls BotchQuest() immediately.
	 * Leave empty if the quest has no item-loss botch condition.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Botch")
	TArray<int32> BotchItemIDs;

	/**
	 * Journal text shown when the quest is botched via BotchItemIDs item loss.
	 * Ignored when BotchItemIDs is empty.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Botch",
		meta = (EditCondition = "BotchItemIDs.Num() > 0", MultiLine = true))
	FText ItemLossBotchDescription;

	/// Convert this asset to an FQuestMetaData struct for runtime use.
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FQuestMetaData ToQuestMetaData() const
	{
		FQuestMetaData Out;
		Out.QuestID = QuestID;
		Out.QuestTitle = QuestTitle;
		Out.QuestDescription = QuestDescription;
		Out.MentionedDescription = MentionedDescription;
		Out.Repeatable = Repeatable;
		Out.Steps = Steps;
		Out.BotchConditions = BotchConditions;
		Out.BotchItemIDs = BotchItemIDs;
		Out.ItemLossBotchDescription = ItemLossBotchDescription;
		return Out;
	}

#if WITH_EDITORONLY_DATA
	/// Editor notes — not included in cooked builds.
	UPROPERTY(EditAnywhere, Category = "Quest|Editor")
	FString EditorNotes;
#endif
};

