// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "Interfaces/QuestGiverInterface.h"
#include "AreaQuestValidator.generated.h"

UCLASS(Blueprintable)
class DIALOGANDQUESTPLUGIN_API AAreaQuestValidator : public AActor, public IQuestGiverInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAreaQuestValidator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UBoxComponent> BoxComponent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UQuestGiverComponent> QuestComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FQuestValidatableSteps ValidatableSteps;

	UFUNCTION()
	void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryProgressQuest(AActor* OtherActor);

public:
	virtual UQuestGiverComponent* GetQuestGiverComponent() const override { return QuestComponent; }
};
