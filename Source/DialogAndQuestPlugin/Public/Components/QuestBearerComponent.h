#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialog/DialogData.h"
#include "Quest/QuestData.h"
#include "QuestBearerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKnownQuestChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQuestUpdated, int64, QuestID, int32, QuestStepID, EQuestState, NewState);

/**
 * Per-player quest state component. Lives on the PlayerController (or any actor
 * implementing IQuestBearerInterface).
 *
 * ## Responsibilities
 * - Stores all quests the player knows about (KnownQuestData, replicated to owning client).
 * - Exposes the full quest state machine: Mention → Accept → Progress → Achieve → Complete/Botch.
 * - Provides step-query helpers (IsAtStep, IsPastStep, etc.) used by the dialog condition
 *   system (FDialogTopicCondition::VerifyCondition).
 * - Broadcasts delegates on quest changes so UI (journal, dialog window) can react.
 *
 * ## Delegates
 * - KnownQuestDispatcher: Fires on any quest data change (replication callback).
 * - NewQuestDispatcher: Fires when a previously unknown quest appears in the list.
 * - QuestUpdateDispatcher (FQuestUpdated): Fires on every state/step transition with
 *   (QuestID, StepID, NewState). The game hooks this to persist quest progress to the backend.
 *
 * ## Dialog Integration
 * The dialog system calls CanDisplay() and GetQuestState() (via IQuestBearerInterface)
 * during FDialogTopicCondition::VerifyCondition(). After every topic click, the dialog
 * window calls RefreshDialogOptions() which re-evaluates all conditions — so quest-gated
 * topics appear/disappear immediately as state changes.
 *
 * ## Step Progression Flow
 *   1. Player clicks a dialog topic with QuestRelation or Consequence
 *   2. TryProgressQuest() → Server RPC → QuestMainComponent::TryProgressQuest()
 *   3. QuestMainComponent validates via QuestGiverComponent::CanValidateQuestStep()
 *   4. If valid: QuestBearerComponent::ProgressQuest() advances step, broadcasts QuestUpdateDispatcher
 *   5. Dialog window receives KnownQuestDispatcher → RefreshDialogOptions() → topics update
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIALOGANDQUESTPLUGIN_API UQuestBearerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQuestBearerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_KnownQuest, BlueprintReadWrite)
	TArray<FQuestProgressData> KnownQuestData;

	UPROPERTY(BlueprintReadWrite)
	TMap<int64, int32> KnownQuestDataLUT;

	/// Rebuild the LUT from the KnownQuestData array. Call after any mutation.
	void RebuildQuestLUT();

	UFUNCTION()
	void OnRep_KnownQuest();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryProgressQuest(int64 QuestID, AActor* Validator);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryProgressAll(AActor* Validator);

public:

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool Authority_TryProgressQuest(int64 QuestID, AActor* Validator);

	const TArray<FQuestProgressData>& GetAllKnownQuest() const { return KnownQuestData; }

	/// Returns reference to the quest. Caller MUST check IsQuestKnown() first — asserts in debug.
	const FQuestProgressData& GetKnownQuest(int64 QuestID) const;

	/// Null-safe variant — returns nullptr if the quest is not known. Prefer this in code paths
	/// where existence is not guaranteed (e.g., inside RPC handlers).
	const FQuestProgressData* GetKnownQuestSafe(int64 QuestID) const;

	bool IsQuestKnown(int64 QuestID) const;

	bool CanDisplay(int64 QuestID, int32 StepID, EQuestStepConditionType Condition) const;
	
	bool CanValidate(int64 QuestID, int32 StepID) const;

	bool IsBeforeStep(int64 QuestID, int32 StepID) const;

	bool IsBeforeOrAtStep(int64 QuestID, int32 StepID) const;

	bool IsPastStep(int64 QuestID, int32 StepID) const;
	
	bool IsAtOrPastStep(int64 QuestID, int32 StepID) const;
	
	bool IsAtStep(int64 QuestID, int32 StepID) const;

	UPROPERTY(BlueprintAssignable) //this is public because its a dispatcher
	FKnownQuestChanged KnownQuestDispatcher;

	UPROPERTY(BlueprintAssignable) //this is public because its a dispatcher
	FKnownQuestChanged NewQuestDispatcher;

	UPROPERTY(BlueprintAssignable)
	FQuestUpdated QuestUpdateDispatcher;

	//------------------------------------------------------------------------------------------------------------------
	// State machine transitions
	//------------------------------------------------------------------------------------------------------------------

	/// Mention a quest — transitions from Unknown to Mentioned. Adds a journal rumor entry.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest|State")
	void MentionQuest(int64 QuestID);

	/// Brief a quest — transitions Unknown→Briefed or Mentioned→Briefed.
	/// Player has spoken to the quest giver and heard the pitch but not yet committed.
	/// No-ops if already Briefed or further along the state machine.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest|State")
	void BriefQuest(int64 QuestID);

	/// Accept a quest — transitions from Unknown/Mentioned/Briefed to Accepted. Starts step tracking.
	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	void AuthorityAddQuest(int64 QuestID);

	/// Mark a quest as Achieved (objectives done, awaiting turn-in).
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest|State")
	void AchieveQuest(int64 QuestID);

	/// Mark a quest as Completed (fully turned in).
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest|State")
	void CompleteQuest(int64 QuestID);

	/// Botch a quest — irrecoverable failure.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Quest|State")
	void BotchQuest(int64 QuestID);

	/// Get the current state of a quest.
	UFUNCTION(BlueprintCallable, Category = "Quest|State")
	EQuestState GetQuestState(int64 QuestID) const;

	//------------------------------------------------------------------------------------------------------------------
	// Legacy / step-based progression
	//------------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AuthoritySetupQuestData(int64 QuestID, int32 StepID, EQuestState InitialState = EQuestState::Accepted);

	UFUNCTION(BlueprintCallable)
	void TryProgressQuest(int64 QuestID, AActor* Validator);

	UFUNCTION(BlueprintCallable)
	void TryProgressAll(AActor* Validator);

	/// @param bSilent  When true, QuestUpdateDispatcher is NOT broadcast. Use during load-time
	///                step replay to avoid triggering N save calls on login.
	void ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep,
	                   bool SkipReward = false, bool bSilent = false);

	void AddQuest(const FQuestMetaData& QuestMeta);

	UFUNCTION(BlueprintCallable)
	bool CanValidateStepWithItems(int64 QuestID, int32 StepID, const TArray<int32>& InputItems, float InputCoins,
		TArray<int32>& OutputItems, float& OutputCoins);

	bool RemoveQuestData(int64 QuestID, bool bBroadcast = true);
	void ClearAllQuestData(bool bBroadcast = true);
	void ReplaceQuestData(const TArray<FQuestProgressData>& NewQuestData, bool bBroadcast = true);
	bool AuthoritySetQuestData(int64 QuestID, int32 StepID, EQuestState NewState, bool bBroadcast = true);

#if WITH_AUTOMATION_TESTS
	/**
	 * Test-only: directly inject a pre-built FQuestProgressData entry into KnownQuestData
	 * and rebuild the LUT.  Bypasses the ROLE_Authority gate so unit tests running outside
	 * a full game world can populate quest state without a server.
	 * NOT compiled into shipping or development builds.
	 */
	void Test_InjectQuestData(FQuestProgressData Data)
	{
		KnownQuestData.Add(MoveTemp(Data));
		RebuildQuestLUT();
	}

	void Test_ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep,
	                        bool SkipReward = false, bool bSilent = false)
	{
		ApplyQuestProgress(QuestMeta, NextQuestStep, SkipReward, bSilent);
	}
#endif

private:
	void ApplyQuestProgress(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep,
	                        bool SkipReward, bool bSilent);
	void PopulateBranchAlternatives(FQuestProgressData& QuestData, const FQuestMetaData& QuestMeta) const;
};
