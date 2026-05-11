#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundBase.h"
#include "UObject/Object.h"
#include "QuestData.generated.h"

/**
 * QuestData.h — Quest data structures, state machine, and step definitions.
 *
 * ## Quest State Machine
 *
 *   Unknown ──MentionQuest()──► Mentioned ──AuthorityAddQuest()──► Accepted
 *                                   │                                 │
 *   Unknown ──AuthorityAddQuest()───┘──────────────────────────►      │
 *                                                                     │
 *                               ┌──────── ProgressQuest() ───────────┘
 *                               │              (step by step)
 *                               ▼
 *                           Accepted ──AchieveQuest()──► Achieved ──CompleteQuest()──► Completed
 *                               │                            │
 *                               └──BotchQuest()──► Botched ◄─┘
 *
 * - **Unknown**: Player has never encountered this quest.
 * - **Mentioned**: Player heard about the quest (journal shows a rumor). Cannot progress steps.
 * - **Accepted**: Quest is active. Steps are tracked. This is the main "in progress" state.
 * - **Achieved**: All objectives done, awaiting turn-in to the quest giver.
 * - **Completed**: Quest turned in successfully. Terminal state.
 * - **Botched**: Quest failed irrecoverably. Terminal state.
 *
 * ## Step Progression
 *
 * Each FQuestMetaData contains an ordered array of FQuestStep. The player's current step
 * is tracked in FQuestProgressData::ProgressID (matches FQuestStep::QuestSubID).
 *
 * Progression is driven by TryProgressQuest() → QuestMainComponent validates against the
 * QuestGiverComponent → QuestBearerComponent::ProgressQuest() advances the step.
 *
 * ## Key Classes
 *
 * | Class                    | Role                                                     |
 * |--------------------------|----------------------------------------------------------|
 * | FQuestMetaData           | Static quest definition (title, steps, botch conditions) |
 * | FQuestStep               | Single quest objective definition                        |
 * | FQuestProgressData       | Per-player quest progress (state + current step)         |
 * | UQuestMainComponent      | Server-side quest registry (GameMode component)          |
 * | UQuestBearerComponent    | Per-player quest state (PlayerController component)      |
 * | UQuestGiverComponent     | NPC-side: which quest steps this NPC can validate        |
 * | IQuestBearerInterface    | Interface for accessing quest state on PlayerController  |
 *
 * ## Interaction with Dialog
 *
 * Dialog topics can be conditionally shown based on quest state/step via
 * FDialogTopicCondition (see DialogData.h). When a topic is clicked, FDialogConsequence
 * can transition quest state or mention new quests. The dialog window refreshes topic
 * visibility after every click, so quest-gated topics appear/disappear in real time.
 */

UENUM(BlueprintType)
enum struct EQuestStepConditionType: uint8
{
	Equal,
	Lesser,
	LesserEqual,
	Greater,
	GreaterEqual
};

///@brief Quest state machine following a Mentioned→Briefed→Accepted→Achieved→Completed flow with a Botched dead-end.
UENUM(BlueprintType)
enum class EQuestState : uint8
{
	Unknown    UMETA(DisplayName = "Unknown"),
	Mentioned  UMETA(DisplayName = "Mentioned"),
	Briefed    UMETA(DisplayName = "Briefed"),   ///< Player spoke to quest giver, heard pitch — not yet committed. ProgressID = -1.
	Accepted   UMETA(DisplayName = "Accepted"),
	Achieved   UMETA(DisplayName = "Achieved"),
	Completed  UMETA(DisplayName = "Completed"),
	Botched    UMETA(DisplayName = "Botched")
};

///@brief Defines how a quest step connects to subsequent steps.
UENUM(BlueprintType)
enum class EQuestStepType : uint8
{
	Linear   UMETA(DisplayName = "Linear"),
	Branch   UMETA(DisplayName = "Branch"),
	Parallel UMETA(DisplayName = "Parallel"),
	Optional UMETA(DisplayName = "Optional")
};

///@brief Concrete quest reward data. Subclass for game-specific rewards.
UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class UQuestRewardData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float XPReward = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float CoinReward = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TArray<int32> ItemRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FGameplayTag FactionAdjustmentTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	float FactionAdjustmentValue = 0.f;
};

///@brief Represent a quest objective, eg, go to the bakery, talk to somebody, bring 10 wolf fang
USTRUCT(BlueprintType)
struct FQuestStep : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	int32 QuestSubID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FText StepTitle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FText StepDescription;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	bool FinishingStep = false;

	/** DataAsset supplying the reward granted when this step is completed. Null = no reward. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	TObjectPtr<UQuestRewardData> RewardAsset;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Requirements")
	TArray<int32> NecessaryItems;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Requirements")
	float NecessaryCoins = 0.f;

	/**
	 * Optional validator class filter for item turn-in steps.
	 * When set, only an NPC whose class IsChildOf this class will register this step during InitQuest().
	 * Leave empty to allow any NPC that lists this quest in HandledQuests (default, backward-compatible).
	 * Must implement IQuestGiverInterface.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Requirements",
		meta = (MustImplement = "/Script/DialogAndQuestPlugin.QuestGiverInterface"))
	TSubclassOf<UObject> ValidatorClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FText ItemTurnInDialog;

	/// Step type for branching/parallel support. Linear = next step in array order.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Flow")
	EQuestStepType StepType = EQuestStepType::Linear;

	/// Explicit next step IDs. If empty and StepType == Linear, next step is the following array element.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Flow",
		meta = (EditCondition = "StepType != EQuestStepType::Linear"))
	TArray<int32> NextStepIDs;

	/// Voiceover sound to play when this step's turn-in dialog is displayed.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Voiceover")
	TSoftObjectPtr<USoundBase> VoiceoverCue;

	/// External audio middleware event name (Wwise/FMOD).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Voiceover")
	FName VoiceoverEventName;

	/// Manual subtitle duration override (seconds). 0 = use sound asset duration.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Voiceover",
		meta = (ClampMin = "0.0"))
	float VoiceoverDuration = 0.f;
};

///@brief Condition that can botch a quest (abstract concept — game implements the check).
USTRUCT(BlueprintType)
struct FQuestBotchCondition : public FTableRowBase
{
	GENERATED_BODY()

	/// GameplayTag identifying the botch trigger type (e.g. "Quest.Botch.NPCDead", "Quest.Botch.ItemDestroyed").
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Botch")
	FGameplayTag BotchTag;

	/// Description shown in the journal when the quest is botched by this condition.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Botch")
	FText BotchDescription;
};

///@brief Represent the complete quest info
USTRUCT(BlueprintType)
struct FQuestMetaData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FText QuestTitle;

	/// Full quest description shown once the quest is accepted.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FText QuestDescription;

	/// Rumor/mention text shown in the journal when the quest is only "Mentioned" (not yet accepted).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	FText MentionedDescription;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	bool Repeatable = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest")
	TArray<FQuestStep> Steps;

	/// Conditions that can botch (fail) this quest irrecoverably.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Botch")
	TArray<FQuestBotchCondition> BotchConditions;

	/**
	 * Item IDs that must stay in the player's possession (inventory or bank) while this
	 * quest is active. If ANY of these items is dropped, sold, or destroyed,
	 * ANeverQuestGameMode::HandleQuestBotchOnItemLoss() calls BotchQuest() immediately.
	 * Leave empty if the quest has no item-loss botch condition.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Botch")
	TArray<int32> BotchItemIDs;

	/**
	 * Journal text shown when the quest is botched via BotchItemIDs item loss.
	 * Ignored when BotchItemIDs is empty.
	 * Example: "You lost the crate of defective arrows. Guard Weleth will have to find another courier."
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Quest|Botch",
		meta = (EditCondition = "BotchItemIDs.Num() > 0"))
	FText ItemLossBotchDescription;
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

///@brief Represent a quest objective and its completion state (player progress)
USTRUCT(BlueprintType)
struct FQuestProgressStep : public FQuestStep
{
	GENERATED_BODY()
	FQuestProgressStep() = default;

	explicit DIALOGANDQUESTPLUGIN_API FQuestProgressStep(const FQuestStep& Step);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool Completed = false;
};

///@brief Quest from the player perspective with state machine and progress tracking
USTRUCT(BlueprintType)
struct FQuestProgressData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int64 QuestID = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ProgressID = 0;

	UPROPERTY(BlueprintReadOnly)
	EQuestState State = EQuestState::Unknown;

	UPROPERTY(BlueprintReadOnly)
	bool Repeatable = false;

	UPROPERTY(BlueprintReadOnly)
	FQuestProgressStep CurrentStep;

	UPROPERTY(BlueprintReadOnly)
	FText QuestTitle;

	/// Rumor text (only relevant when State == Mentioned).
	UPROPERTY(BlueprintReadOnly)
	FText MentionedDescription;

	///steps already done by the player
	UPROPERTY(BlueprintReadOnly)
	TArray<FQuestProgressStep> PreviousStep;

	/// Full quest description shown once the quest is accepted (mirrors FQuestMetaData::QuestDescription).
	UPROPERTY(BlueprintReadOnly)
	FText QuestDescription;

	/**
	 * Populated when CurrentStep.StepType == Branch.
	 * Contains one FQuestProgressStep per NextStepIDs destination so the journal can display
	 * all available paths separated by "OR" blocks without querying the server-side quest
	 * registry from the client. Cleared automatically when the step is no longer a Branch.
	 */
	UPROPERTY(BlueprintReadOnly)
	TArray<FQuestProgressStep> BranchAlternatives;

	/// True if the quest is in a terminal state (Completed or Botched).
	bool IsTerminal() const { return State == EQuestState::Completed || State == EQuestState::Botched; }

	/// True if the quest is actively being pursued (Accepted or Achieved).
	bool IsActive() const { return State == EQuestState::Accepted || State == EQuestState::Achieved; }

	/// Backward compat helper.
	bool IsFinished() const { return State == EQuestState::Completed; }
};


///@brief Global quest data for journal title display
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

/// Journal display categories — groups EQuestState values into the three book tabs.
UENUM(BlueprintType)
enum class EQuestJournalCategory : uint8
{
	Active   UMETA(DisplayName = "Active"),    ///< Accepted + Achieved
	Finished UMETA(DisplayName = "Finished"),  ///< Completed + Botched
	Rumored  UMETA(DisplayName = "Rumored"),   ///< Mentioned
};

/// Maps a quest state to its journal category.
inline EQuestJournalCategory GetQuestJournalCategory(EQuestState State)
{
	switch (State)
	{
	case EQuestState::Mentioned:
	case EQuestState::Briefed:   return EQuestJournalCategory::Rumored;
	case EQuestState::Completed:
	case EQuestState::Botched:   return EQuestJournalCategory::Finished;
	default:                     return EQuestJournalCategory::Active;
	}
}

///@brief Quest step entry for journal step display
UCLASS(BlueprintType)
class DIALOGANDQUESTPLUGIN_API UQuestEntryData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FQuestProgressStep Data;

	UPROPERTY(BlueprintReadWrite)
	UQuestJournalWindow* Parent = nullptr;

	/**
	 * When true this entry is a visual "— OR —" separator between branch alternatives.
	 * Data is empty; the step widget Blueprint should detect this flag and render a divider
	 * row instead of normal title/description fields.
	 */
	UPROPERTY(BlueprintReadWrite)
	bool bIsBranchSeparator = false;
};
