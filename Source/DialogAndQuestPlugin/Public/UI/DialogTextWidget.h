// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "DialogTextChunkWidget.h"
#include "DialogWindow.h"
#include "Blueprint/UserWidget.h"
#include "DialogTextWidget.generated.h"

/**
 * 
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTextWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly)
	UDialogWindow* ParentDialog = nullptr;

	UPROPERTY(BlueprintReadOnly)
	const UDialogComponent* DialogComponent = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddTopicData(const FDialogTextData& DialogTopic);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ClearList();

public:
	UFUNCTION(BlueprintCallable)
	void AddEmptyTopicData(const FString& DialogText);

	UFUNCTION(BlueprintCallable)
	void InitDialog(UDialogWindow* InputParentDialog);

	UFUNCTION(BlueprintCallable)
	void AddTopicText(int64 TopicID);
};
