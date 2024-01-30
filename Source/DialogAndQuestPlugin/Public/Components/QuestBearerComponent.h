// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialog/DialogData.h"
#include "Quest/QuestData.h"
#include "QuestBearerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKnownQuestChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuestUpdated, int64, QuestID, int32, QuestStepID);

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

	const FQuestProgressData& GetKnownQuest(int64 QuestID) const;

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

	UFUNCTION(BlueprintCallable)
	void AuthorityAddQuest(int64 QuestID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AuthoritySetupQuestData(int64 QuestID, int32 StepID);

	UFUNCTION(BlueprintCallable)
	void TryProgressQuest(int64 QuestID, AActor* Validator);

	UFUNCTION(BlueprintCallable)
	void TryProgressAll(AActor* Validator);

	void ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep, bool SkipReward = false);

	void AddQuest(const FQuestMetaData& QuestMeta);

	UFUNCTION(BlueprintCallable)
	bool CanValidateStepWithItems(int64 QuestID, int32 StepID, const TArray<int32>& InputItems, float InputCoins,
		TArray<int32>& OutputItems, float& OutputCoins);

};
