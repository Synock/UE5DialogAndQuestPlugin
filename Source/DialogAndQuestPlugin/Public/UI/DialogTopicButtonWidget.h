// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "DialogTextChunkWidget.h"
#include "DialogWindow.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "DialogTopicButtonWidget.generated.h"


/**
 * 
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTopicButtonWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	UDialogWindow* ParentDialog = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FDialogTextData LocalData;

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void InitData(const FDialogTextData& ItemData);

	UFUNCTION(BlueprintCallable)
	void InitParent(UDialogWindow* Parent);

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
