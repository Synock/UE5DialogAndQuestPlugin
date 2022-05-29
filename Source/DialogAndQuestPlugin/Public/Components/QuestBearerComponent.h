// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/QuestData.h"
#include "QuestBearerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKnownQuestChanged);

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
<<<<<<< HEAD
	void Server_UpdateQuest(int64 QuestID, int32 QuestStep, AActor* QuestGiver);

	UFUNCTION(BlueprintCallable)
	void UpdateQuest(int64 QuestID, int32 QuestStep, AActor* QuestGiver);

public:
=======
	void Server_TryProgressQuest(int64 QuestID, AActor* Validator);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryProgressAll(AActor* Validator);


public:

>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	const TArray<FQuestProgressData>& GetAllKnownQuest() const { return KnownQuestData; }

	const FQuestProgressData& GetKnownQuest(int64 QuestID) const;

<<<<<<< HEAD
=======
	bool IsQuestKnown(int64 QuestID) const;

	bool CanDisplay(int64 QuestID, int32 StepID) const;
	
	bool CanValidate(int64 QuestID, int32 StepID) const;

	bool IsPastStep(int64 QuestID, int32 StepID) const;
	
	bool IsAtOrPastStep(int64 QuestID, int32 StepID) const;
	
	bool IsAtStep(int64 QuestID, int32 StepID) const;

>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	UPROPERTY(BlueprintAssignable) //this is public because its a dispatcher
	FKnownQuestChanged KnownQuestDispatcher;

	UFUNCTION(BlueprintCallable)
<<<<<<< HEAD
	void TryValidateQuestFromActor(int64 QuestID, AActor* QuestGiver);

	UFUNCTION(BlueprintCallable)
	void AuthorityAddQuest(int64 QuestID);
=======
	void AuthorityAddQuest(int64 QuestID);

	UFUNCTION(BlueprintCallable)
	void TryProgressQuest(int64 QuestID, AActor* Validator);

	UFUNCTION(BlueprintCallable)
	void TryProgressAll(AActor* Validator);

	void ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep);

	void AddQuest(const FQuestMetaData& QuestMeta);

>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
};
