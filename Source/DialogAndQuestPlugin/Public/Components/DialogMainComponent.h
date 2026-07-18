#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialog/DialogData.h"
#include "DialogMainComponent.generated.h"

class UDialogAsset;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DIALOGANDQUESTPLUGIN_API UDialogMainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDialogMainComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/// a topic of conversation eg, weather, skeleton invasion...
	UPROPERTY(BlueprintReadWrite)
	TMap<int64, FDialogTopicStruct> DialogTopic;

	/// a lookup table that link topic with
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, int64> DialogTopicLUT;

	///Topic bundle, eg information about local city or quest dialog
	UPROPERTY(BlueprintReadWrite)
	TMap<int64, FDialogTopicBundleStruct> DialogBundle;

	///Meta bundle, that link several bundle, eg, information about local city, information about armor, quest data
	UPROPERTY(BlueprintReadWrite)
	TMap<int64, FDialogTopicMetaBundleStruct> DialogMetaBundle;


public:

	UFUNCTION(BlueprintCallable)
	void AddTopicFromDataTable(UDataTable* DataTable);

	UFUNCTION(BlueprintCallable)
	void AddBundleFromDataTable(UDataTable* DataTable);

	UFUNCTION(BlueprintCallable)
	void AddMetaBundleFromDataTable(UDataTable* DataTable);

	/// Load all topics from a UDialogAsset. Replaces the DataTable pipeline.
	UFUNCTION(BlueprintCallable)
	void AddFromAsset(UDialogAsset* DialogAsset);

	UFUNCTION(BlueprintCallable)
	void AddTopic(const FDialogTopicStruct& NewTopic);

	UFUNCTION(BlueprintCallable)
	void AddBundle(const FDialogTopicBundleStruct& Bundle);

	UFUNCTION(BlueprintCallable)
	void AddMetaBundle(const FDialogTopicMetaBundleStruct& MetaBundle);

	UFUNCTION(BlueprintCallable)
	TArray<FDialogTopicStruct> GetAllDialogTopicForBundle(int64 BundleId) const;

	UFUNCTION(BlueprintCallable)
	TArray<FDialogTopicStruct> GetAllDialogTopicForMetaBundle(int64 BundleMetaId) const;

	/// Returns true if a MetaBundle with the given ID has been registered.
	/// Used by InitDialogFromID to distinguish "MetaBundle not yet loaded" from "loading succeeded".
	UFUNCTION(BlueprintCallable)
	bool HasMetaBundle(int64 MetaBundleId) const { return DialogMetaBundle.Contains(MetaBundleId); }

	UFUNCTION(BlueprintCallable)
	FText GetBadGreeting(int64 BundleMetaId) const;

	UFUNCTION(BlueprintCallable)
	FText GetGoodGreeting(int64 BundleMetaId) const;

	UFUNCTION(BlueprintCallable)
	float GetGreetingRelationLimit(int64 BundleMetaId) const;

	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<USoundBase> GetGoodGreetingVoiceover(int64 BundleMetaId) const;

	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<USoundBase> GetBadGreetingVoiceover(int64 BundleMetaId) const;

	TArray<FConditionalGreeting> GetConditionalGreetings(int64 BundleMetaId) const;
};
