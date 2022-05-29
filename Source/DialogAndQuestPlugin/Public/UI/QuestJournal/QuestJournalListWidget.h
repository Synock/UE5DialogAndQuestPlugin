// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/QuestBearerComponent.h"
#include "Quest/QuestData.h"
#include "QuestJournalListWidget.generated.h"

/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalListWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	class UQuestJournalWindow* ParentJournal = nullptr;

	UPROPERTY(BlueprintReadOnly)
	const UQuestBearerComponent* QuestBearerComponent = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddQuestTitleData(const FQuestProgressData& QuestData);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ClearList();


public:
	UFUNCTION(BlueprintCallable)
	void InitDialog(UQuestJournalWindow* InputParentDialog);

	UFUNCTION(BlueprintCallable)
	void UpdateQuestList();
};
