#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Dialog/DialogData.h"
#include "DialogAsset.generated.h"

/**
 * Unified dialog asset that replaces the Topic→Bundle→MetaBundle three-table chain.
 * One asset per NPC dialog configuration (or shared among NPCs with same dialog).
 * NPCs reference this via TObjectPtr<UDialogAsset> instead of int64 IDs.
 */
UCLASS(BlueprintType)
class DIALOGANDQUESTPLUGIN_API UDialogAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/// Display name for editor identification.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString AssetName;

	/// Greeting when player relation >= MinimumRelation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Greeting")
	FText GoodGreeting = FText::FromString("Greetings");

	/// Greeting when player relation < MinimumRelation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Greeting")
	FText BadGreeting = FText::FromString("I don't wish to speak to your kind. Now get lost!");

	/// Relation threshold for good/bad greeting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Greeting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinimumRelation = 0.375f;

	/// Voiceover for the good greeting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Greeting|Voiceover")
	TSoftObjectPtr<USoundBase> GoodGreetingVoiceover;

	/// Voiceover for the bad greeting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Greeting|Voiceover")
	TSoftObjectPtr<USoundBase> BadGreetingVoiceover;

	/// All dialog topics owned by this asset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Topics")
	TArray<FDialogTopicStruct> Topics;

	/**
	 * Shared topic assets whose topics are merged into this NPC's dialog.
	 *
	 * Use this to compose dialog from reusable building blocks:
	 *   - DA_CityLore   — shared world lore, used by every NPC in the city
	 *   - DA_QuestGiver — shared "I have work for you" opener
	 *
	 * At runtime, AddFromAsset() processes each shared asset first (idempotent — safe to
	 * list the same shared asset in multiple NPC assets), then merges their bundle IDs into
	 * this asset's MetaBundle. The NPC will see topics from shared assets AND its own Topics[].
	 *
	 * Shared assets are loaded synchronously on the server during StartPlay().
	 * Do NOT create circular references.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog|Shared")
	TArray<TSoftObjectPtr<UDialogAsset>> SharedTopicAssets;

#if WITH_EDITORONLY_DATA
	/// Editor notes — not included in cooked builds.
	UPROPERTY(EditAnywhere, Category = "Dialog|Editor")
	FString EditorNotes;
#endif
};

