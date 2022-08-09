// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Quest/QuestData.h"
#include "QuestJournalStepWidget.generated.h"

/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UQuestJournalStepWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void InitData(const FQuestProgressStep& ItemData);

	UPROPERTY(BlueprintReadOnly)
	int64 ItemID = 0;

	UPROPERTY(BlueprintReadOnly)
	FQuestProgressStep LocalData;

	UPROPERTY(BlueprintReadOnly)
	UQuestJournalWindow* ParentJournal = nullptr;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UFUNCTION(BlueprintCallable)
	static FString GetStrike(const FString& OriginalString);

	UFUNCTION(BlueprintCallable)
	static FString GetBold(const FString& OriginalString);
};
