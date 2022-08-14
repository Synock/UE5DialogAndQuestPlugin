// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "DialogTextChunkWidget.h"
#include "DialogWindow.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
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

	UPROPERTY(BlueprintReadWrite)
	UListView* ListViewWidget = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddTopicData(const FDialogTextData& DialogTopic);

	UFUNCTION(BlueprintCallable)
	FString ProcessText(const FString & InputString) const;

public:

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ClearList();

	UFUNCTION(BlueprintCallable)
	void AddEmptyTopicData(const FString& DialogText);

	UFUNCTION(BlueprintCallable)
	void InitDialog(UDialogWindow* InputParentDialog);

	UFUNCTION(BlueprintCallable)
	void AddTopicText(int64 TopicID);

	//Reprocess the already displayed text to find new Hyperlinks
	UFUNCTION(BlueprintCallable)
	void ReprocessTopicLinks();
};
