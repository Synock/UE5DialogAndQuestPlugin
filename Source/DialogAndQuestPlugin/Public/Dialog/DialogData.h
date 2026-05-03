#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundBase.h"
#include "Quest/QuestAsset.h"   // replaces direct QuestData.h — covers all quest types + UQuestAsset
#include "DialogData.generated.h"

/**
 * DialogData.h — Dialog topic definitions and quest-driven topic filtering.
 *
 * ## Architecture Overview
 *
 * Dialog is organized as a three-level hierarchy:
 *   MetaBundle → Bundle(s) → Topic(s)
 *
 * Each NPC references a single MetaBundle ID. The MetaBundle aggregates multiple Bundles
 * (e.g., "city lore", "quest dialog", "trade offers"), each of which contains a list of
 * Topic IDs. When dialog opens, all topics from every bundle in the NPC's MetaBundle are
 * loaded into the NPC's UDialogComponent.
 *
 * ## Quest-Driven Topic Visibility
 *
 * Each FDialogTopicStruct carries an FDialogTopicCondition that gates whether the topic
 * is shown to the player. The dialog UI calls VerifyCondition() every time the topic list
 * refreshes — which happens on dialog open, after every topic click, and when the quest
 * journal updates. This means the same NPC can show different topics as a quest progresses.
 *
 * ### Condition Types (can be combined)
 *
 * | Field               | Purpose                                                       |
 * |---------------------|---------------------------------------------------------------|
 * | QuestId             | Quest to check. 0 = no quest filter.                          |
 * | RequiredQuestState  | Show only when quest is in this state. Unknown = no filter.   |
 * | MinimumStepID       | Step-based filter. 0 = no step filter.                        |
 * | StepCondition       | Comparison operator for step check (Equal, Greater, etc.).    |
 * | MinimumRelation     | Minimum NPC→player relation to show topic.                    |
 * | RequiredItems       | Items the player must carry.                                  |
 * | SkillCheckTag       | GameplayTag for a skill check (via IDialogSkillCheckInterface)|
 * | bConsumeOnUse       | If true, topic disappears after first click.                  |
 *
 * State and step filters can be combined: if both RequiredQuestState and MinimumStepID are
 * set, the topic only appears when BOTH conditions are satisfied.
 *
 * ### Example: Multi-Step Quest Dialog
 *
 *   Topic "Tell me about wolves"
 *     → QuestId=0 (always visible, relation permitting)
 *
 *   Topic "I'll hunt the wolves"   (quest-accepting topic)
 *     → QuestId=100, RequiredQuestState=Unknown  → never shown (quest must be known)
 *     → Better: no QuestId, use Consequence to set quest to Accepted
 *
 *   Topic "I killed the wolves"    (mid-quest turn-in)
 *     → QuestId=100, RequiredQuestState=Accepted, MinimumStepID=2, StepCondition=Equal
 *     → Appears only when quest 100 is Accepted AND player is on step 2
 *
 *   Topic "Thank you, hero"        (post-completion)
 *     → QuestId=100, RequiredQuestState=Completed
 *     → Appears only after quest 100 is completed
 *
 * ## Consequence System
 *
 * When a topic is clicked, FDialogConsequence fires:
 *   - Transition a quest to a new state (NewQuestState)
 *   - Advance a quest step without changing state (bAdvanceStep = true, NewQuestState = Unknown)
 *   - Both at once: bAdvanceStep = true + NewQuestState = Achieved
 *   - Mention a new quest (MentionQuestID → sets quest to Mentioned state)
 *   - Adjust faction relation (FactionDelta)
 *
 * Step advancement (bAdvanceStep) uses the dialog NPC as the quest validator — the NPC's
 * QuestGiverComponent must have the relevant step registered via AddValidatableSteps().
 *
 * The PlayerController must implement IDialogConsequenceInterface to handle these.
 *
 */

///@brief This represents a condition for a dialog to appear.
/// Supports quest-state gating, relation checks, item requirements, and skill checks.
USTRUCT(BlueprintType)
struct DIALOGANDQUESTPLUGIN_API FDialogTopicCondition  : public FTableRowBase
{
	GENERATED_BODY()

	/// Quest asset this condition checks against. Null = no quest condition.
	/// Replaces the deprecated numeric QuestId field — pick the asset from the picker.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Quest")
	TObjectPtr<UQuestAsset> Quest = nullptr;

	/// Returns the effective quest ID: Quest->QuestID if an asset is assigned,
	/// otherwise falls back to QuestId_DEPRECATED for backward compatibility.
	int64 GetQuestID() const { return Quest ? Quest->QuestID : 0; }

	/// Minimum step ID for step-based condition (legacy). Used with StepCondition.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Quest")
	int32 MinimumStepID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Quest")
	EQuestStepConditionType StepCondition = EQuestStepConditionType::Equal;

	/// If set, topic only shows when the player's quest is in this state.
	/// Unknown = no state filter (fall back to step-based check).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Quest")
	EQuestState RequiredQuestState = EQuestState::Unknown;

	/// Minimum relation value for this topic to appear.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Relation")
	float MinimumRelation = 0.375f;

	/// Items the player must carry (inventory or bank) for this topic to appear. Empty = no item requirement.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Inventory")
	TArray<int32> RequiredItems;

	/**
	 * Items the player must NOT possess (neither inventory nor bank) for this topic to appear.
	 * Use to show a recovery topic only when the player has lost a quest-critical item.
	 * The check is the inverse of RequiredItems: topic hides if the player has ANY of these items.
	 * Empty = no absent-item condition.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Inventory")
	TArray<int32> AbsentItems;

	/// Skill check tag — game implements evaluation via IDialogSkillCheckInterface.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Skill")
	FGameplayTag SkillCheckTag;

	/// Minimum skill value required (only used when SkillCheckTag is valid).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition|Skill",
		meta = (EditCondition = "SkillCheckTag.IsValid()"))
	float MinimumSkillValue = 0.f;

	/// If true, this topic disappears after the player clicks it once.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Condition")
	bool bConsumeOnUse = false;

	bool VerifyCondition(const AActor* DialogActor, const APlayerController* Controller) const;
};

///@brief Consequence triggered when the player clicks a dialog topic.
USTRUCT(BlueprintType)
struct FDialogConsequence : public FTableRowBase
{
	GENERATED_BODY()

	/// Quest asset to transition or advance. Null = no quest consequence.
	/// Replaces the deprecated numeric QuestID field — pick the asset from the picker.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Consequence|Quest")
	TObjectPtr<UQuestAsset> Quest = nullptr;

	/// Returns the effective quest ID for state transitions / step advancement.
	int64 GetQuestID() const { return Quest ? Quest->QuestID : 0; }

	/// New quest state to set when this topic is clicked.
	/// Unknown = no state change.
	/// Combine with bAdvanceStep = true to advance a step AND change state in one click.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Consequence|Quest")
	EQuestState NewQuestState = EQuestState::Unknown;

	/// When true (and QuestID != 0), progresses the quest to its next step using the
	/// dialog NPC as the validator. The NPC's QuestGiverComponent must have the target
	/// step registered via AddValidatableSteps().
	/// Set NewQuestState = Unknown to advance the step without touching the quest state.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Consequence|Quest")
	bool bAdvanceStep = false;

	/// Faction relation delta applied when this topic is clicked. 0 = no change.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Consequence|Faction")
	float FactionDelta = 0.f;

	/// Quest asset to "mention" (set to Mentioned state). Null = none.
	/// Implements quest discovery through conversation.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Consequence|Quest")
	TObjectPtr<UQuestAsset> MentionQuest = nullptr;

	/// Returns the effective mention-quest ID.
	int64 GetMentionQuestID() const { return MentionQuest ? MentionQuest->QuestID : 0; }

	/**
	 * Item IDs given directly to the player when this topic is clicked.
	 * Processed by AMainPlayerController::Server_HandleDialogConsequence_Implementation via
	 * PlayerTryAutoLootFunction — items go to inventory (equip slot or bag slot as available).
	 * Use for NPC item recovery topics (re-granting a lost quest item).
	 * Empty = no items granted.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Consequence|Inventory")
	TArray<int32> ItemsToGrant;

	bool HasConsequence() const
	{
		return GetQuestID() != 0 || FactionDelta != 0.f || GetMentionQuestID() != 0 || !ItemsToGrant.IsEmpty();
	}
};

///@brief This struct represent a dialog topic and its potential quest relation
USTRUCT(BlueprintType)
struct FDialogTopicStruct : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog")
	int64 Id = 0;

	/// The keyword (or multi-word phrase) that the player clicks on in dialog text.
	/// The hyperlink scanner performs a longest-match search, so multi-word topics
	/// such as "lost sword" are matched before any single-word sub-key.
	/// Matching is case-insensitive; the original casing from the dialog text is
	/// preserved in the displayed hyperlink.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog")
	FString Topic;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog")
	FDialogTopicCondition TopicCondition;

	/// The NPC's response text when this topic is selected.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog")
	FText TopicText;


	/// New consequence system — triggered when this topic is clicked.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog|Consequence")
	FDialogConsequence Consequence;

	/// Voiceover sound to play when this topic's text is displayed.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog|Voiceover")
	TSoftObjectPtr<USoundBase> VoiceoverCue;

	/// External audio middleware event name (Wwise/FMOD).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog|Voiceover")
	FName VoiceoverEventName;

	/// Manual subtitle duration override (seconds). 0 = use sound asset duration.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dialog|Voiceover",
		meta = (ClampMin = "0.0"))
	float VoiceoverDuration = 0.f;
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Greeting")
	FText GoodGreetingDialog = FText::FromString("Greetings");

	///this is the greeting dialog when player relation is < MinimumRelation
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Greeting")
	FText BadGreetingDialog = FText::FromString("I don't wish to speak to your kind. Now get lost!");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Greeting")
	float MinimumRelation = 0.375;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<int64> TopicBundleList;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString MetaName;

	/// Voiceover for the good greeting.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Greeting|Voiceover")
	TSoftObjectPtr<USoundBase> GoodGreetingVoiceover;

	/// Voiceover for the bad greeting.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Greeting|Voiceover")
	TSoftObjectPtr<USoundBase> BadGreetingVoiceover;
};

