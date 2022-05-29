// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/QuestBearerComponent.h"
#include "QuestJournalDetailsWidget.generated.h"

/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalDetailsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
<<<<<<< HEAD

=======
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
	UPROPERTY(BlueprintReadOnly)
	class UQuestJournalWindow* ParentJournal = nullptr;

	UPROPERTY(BlueprintReadOnly)
	const UQuestBearerComponent* QuestBearerComponent = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void InitDialog(UQuestJournalWindow* InputParentDialog);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddQuestData(const FQuestProgressData& QuestProgress);

	UFUNCTION(BlueprintCallable)
	void DisplayQuestData(int64 QuestID);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ClearData();
<<<<<<< HEAD

=======
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
};
