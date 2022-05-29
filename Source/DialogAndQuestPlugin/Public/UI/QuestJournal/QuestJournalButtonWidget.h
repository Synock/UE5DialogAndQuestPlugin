// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Quest/QuestData.h"
#include "QuestJournalButtonWidget.generated.h"

/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalButtonWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	class UQuestJournalWindow* ParentJournal = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FQuestProgressData LocalData;

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void InitData(const FQuestProgressData& ItemData);

	UFUNCTION(BlueprintCallable)
	void InitParent(UQuestJournalWindow* Parent);

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
