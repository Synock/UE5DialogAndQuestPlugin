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

#if WITH_EDITORONLY_DATA
	/// Editor notes — not included in cooked builds.
	UPROPERTY(EditAnywhere, Category = "Dialog|Editor")
	FString EditorNotes;
#endif
};

