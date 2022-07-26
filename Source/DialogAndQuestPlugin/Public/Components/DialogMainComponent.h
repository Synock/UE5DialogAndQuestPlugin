// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialog/DialogData.h"
#include "DialogMainComponent.generated.h"


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

	UFUNCTION(BlueprintCallable)
	FString GetBadGreeting(int64 BundleMetaId) const;

	UFUNCTION(BlueprintCallable)
	FString GetGoodGreeting(int64 BundleMetaId) const;

	UFUNCTION(BlueprintCallable)
	float GetGreetingRelationLimit(int64 BundleMetaId) const;
};
