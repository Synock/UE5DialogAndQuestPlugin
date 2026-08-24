#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialog/DialogAsset.h"
#include "Dialog/DialogData.h"
#include "DialogComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoiceoverRequested, USoundBase*, SoundCue, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVoiceoverStop);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiddlewareVoiceoverRequested, FName, EventName);

/**
 * Per-NPC dialog data component. Holds the full set of topics this NPC can discuss.
 *
 * ## Lifecycle
 * 1. On server: GameMode calls InitDialogFromID(MetaBundleID) which loads all topics
 *    from UDialogMainComponent (the server-side registry).
 * 2. DialogTopicData replicates to the owning client.
 * 3. On client: OnRep_DialogData rebuilds the local lookup maps.
 *
 * ## Topic Filtering
 * All topics are stored unconditionally in this component. Filtering happens at display
 * time: UDialogTopicWidget::UpdateTopicData() iterates every topic and calls
 * FDialogTopicCondition::VerifyCondition() to decide visibility. This is re-run after
 * every topic click via UDialogWindow::RefreshDialogOptions(), so quest-gated topics
 * dynamically appear/disappear during conversation.
 *
 * ## Hyperlink Parsing
 * ParseTextHyperlink() scans NPC response text for words matching topic keywords and
 * wraps them in clickable hyperlinks — but only if that topic's condition passes.
 * This means hyperlinks also respect quest state: a keyword for a quest-locked topic
 * won't be clickable until the quest reaches the required state/step.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIALOGANDQUESTPLUGIN_API UDialogComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDialogComponent();

	UFUNCTION(BlueprintCallable)
	FString GetDialogName() const { return DialogName; }

	UFUNCTION(BlueprintCallable)
	TArray<FDialogTopicStruct> GetAllDialogTopic() const { return DialogTopicData; }


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(ReplicatedUsing=OnRep_DialogData, BlueprintReadOnly)
	TArray<FDialogTopicStruct> DialogTopicData;

	UPROPERTY(BlueprintReadOnly)
	TMap<int64, FDialogTopicStruct> DialogTopic;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, int64> DialogTopicLUT;

	UPROPERTY(Replicated, BlueprintReadWrite)
	FString DialogName;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FText GoodGreeting;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FText BadGreeting;

	UPROPERTY(Replicated, BlueprintReadWrite)
	float GreetingLimit = 0.f;

	/// Tracks whether dialog has been successfully initialized (replaces magic-string check).
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bDialogInitialized = false;

	/// Voiceover cues for greetings (set server-side, reconstructed on client via OnRep).
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> GoodGreetingVoiceover;

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> BadGreetingVoiceover;

	/// Replicated string paths for greeting voiceovers.
	/// TSoftObjectPtr does not replicate directly; we carry the path as FString and
	/// reconstruct the soft ptr on the client inside OnRep_GreetingVoiceovers().
	UPROPERTY(ReplicatedUsing=OnRep_GreetingVoiceovers, BlueprintReadOnly)
	FString GoodGreetingVoiceoverPath;

	UPROPERTY(ReplicatedUsing=OnRep_GreetingVoiceovers, BlueprintReadOnly)
	FString BadGreetingVoiceoverPath;

	/// Ordered generic rules replicated to clients, where the owning player's state is available.
	UPROPERTY(ReplicatedUsing=OnRep_ConditionalGreetings, BlueprintReadOnly)
	TArray<FConditionalGreeting> ConditionalGreetings;

	UFUNCTION()
	void OnRep_GreetingVoiceovers();

	UFUNCTION()
	void OnRep_ConditionalGreetings();

	UFUNCTION()
	virtual void OnRep_DialogData();

public:
	UFUNCTION(BlueprintCallable)
	void InitDialogFromID(int64 ID);

	/**
	 * Primary init path for asset-based NPCs.
	 * Reads greetings and topics directly from the asset (and any SharedTopicAssets),
	 * with no dependency on UDialogMainComponent or GameMode registration.
	 *
	 * @param Asset              The dialog asset to load.
	 * @param FactionGreetingLimit  Faction-wide default greeting limit [0,1].
	 *        Used when Asset::MinimumRelation == -1 (sentinel = "use faction default").
	 *        Pass a negative value to always use the asset's own MinimumRelation.
	 */
	UFUNCTION(BlueprintCallable)
	void InitDialogFromAsset(UDialogAsset* Asset, float FactionGreetingLimit = -1.f);

	/// C++-only lookup — returns nullptr when the topic ID is not found.
	const FDialogTopicStruct* GetDialogTopicSafe(int64 ID) const;

	/// Blueprint-friendly lookup — returns the topic by value; bFound is false when the ID is missing.
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	FDialogTopicStruct GetDialogTopicByID(int64 ID, bool& bFound) const;

	UFUNCTION(BlueprintCallable)
	int64 GetDialogTopicID(const FString& ID) const;

	UFUNCTION(BlueprintCallable)
	FString ParseTextHyperlink(const FString& OriginalString, const AActor* DialogActor, const APlayerController* Controller) const;

	UFUNCTION(BlueprintCallable)
	const FText& GetGoodGreeting() const { return GoodGreeting; }

	UFUNCTION(BlueprintCallable)
	const FText& GetBadGreeting() const { return BadGreeting; }

	UFUNCTION(BlueprintCallable)
	float GetGreetingLimit() const { return GreetingLimit; }

	UFUNCTION(BlueprintCallable)
	bool IsValid() const { return bDialogInitialized; }

	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<USoundBase> GetGoodGreetingVoiceover() const { return GoodGreetingVoiceover; }

	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<USoundBase> GetBadGreetingVoiceover() const { return BadGreetingVoiceover; }

	/** Returns the highest-precedence matching conditional greeting, if any. */
	bool FindConditionalGreeting(const AActor* DialogActor, const APlayerController* Controller,
		FText& OutText, TSoftObjectPtr<USoundBase>& OutVoiceover) const;

	/// Fired when a topic with a voiceover cue is displayed.
	UPROPERTY(BlueprintAssignable)
	FOnVoiceoverRequested OnVoiceoverRequested;

	/// Fired when an ongoing voiceover should be stopped (dialog closed or new topic clicked).
	UPROPERTY(BlueprintAssignable)
	FOnVoiceoverStop OnVoiceoverStop;

	/// Fired alongside OnVoiceoverRequested when the topic has a middleware event name set
	/// (Wwise/FMOD). Bind this instead of OnVoiceoverRequested when using external audio.
	UPROPERTY(BlueprintAssignable)
	FOnMiddlewareVoiceoverRequested OnMiddlewareVoiceoverRequested;

	/// Mark a topic as consumed (for bConsumeOnUse topics).
	UFUNCTION(BlueprintCallable)
	void ConsumeTopicByID(int64 TopicID);

private:
	/// Recursively appends topics from Asset and its SharedTopicAssets into this component's maps.
	/// Does not touch greetings — the primary asset owns those.
	/// Visited is threaded through recursion to prevent infinite loops from circular asset references.
	void AddTopicsFromAsset(UDialogAsset* Asset, TSet<UDialogAsset*>& Visited);

	/// Sorted topic keyword cache for longest-match hyperlink parsing.
	/// Rebuilt lazily whenever DialogTopicLUT changes.
	mutable TArray<FString> SortedTopicKeys;
	mutable bool bTopicKeysDirty = true;

	/// Marks the sorted-key cache as stale. Call whenever DialogTopicLUT is modified.
	void MarkTopicKeysDirty() { bTopicKeysDirty = true; }

	/// Rebuilds SortedTopicKeys from DialogTopicLUT if bTopicKeysDirty is set.
	/// Keys are sorted longest-first so multi-word phrases beat any sub-word match.
	void RebuildSortedTopicKeysIfNeeded() const;
};
