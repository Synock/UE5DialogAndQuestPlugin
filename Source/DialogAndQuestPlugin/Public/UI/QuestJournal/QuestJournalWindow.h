// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "QuestJournalDetailsWidget.h"
#include "QuestJournalListWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/QuestBearerComponent.h"
#include "QuestJournalWindow.generated.h"

/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalWindow : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	UQuestBearerComponent* QuestComponent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UQuestJournalListWidget* ListWidget = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UQuestJournalDetailsWidget* DetailsWidget = nullptr;

	UPROPERTY(BlueprintReadWrite)
	int64 CurrentQuestID = 0;

	UFUNCTION(BlueprintCallable)
	void UpdateKnownQuest();


public:
	UFUNCTION(BlueprintCallable)
	const UQuestBearerComponent* GetQuestComponent() const { return QuestComponent; }

	UFUNCTION(BlueprintCallable)
	void InitJournal();

	UFUNCTION(BlueprintCallable)
	void DisplayQuest(int64 ID);
};
