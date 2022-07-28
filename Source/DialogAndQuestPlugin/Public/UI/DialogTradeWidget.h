// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogTradeWidget.generated.h"

class UDialogComponent;
class UDialogWindow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseTrade);

/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTradeWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly)
	UDialogWindow* ParentDialog = nullptr;

	UPROPERTY(BlueprintReadOnly)
	const UDialogComponent* DialogComponent = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void InitDialog(UDialogWindow* InputParentDialog);

	UPROPERTY(BlueprintAssignable)
	FOnCloseTrade OnClose;
};
