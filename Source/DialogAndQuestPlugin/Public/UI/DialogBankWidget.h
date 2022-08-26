// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Object.h"
#include "DialogBankWidget.generated.h"


class UDialogComponent;
class UDialogWindow;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseBank);
/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogBankWidget : public UUserWidget
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

	//Internal function to do stuff when the widget is displayed
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void DoOnDisplay();

	UPROPERTY(BlueprintAssignable)
	FOnCloseBank OnClose;
};
