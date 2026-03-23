#pragma once

#include "CoreMinimal.h"
#include "DialogWindow.h"
#include "Blueprint/UserWidget.h"
#include "DialogGiveWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelEvent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGiveEvent);

/**
 * 
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogGiveWidget : public UUserWidget
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
	FOnGiveEvent OnGive;

	UPROPERTY(BlueprintAssignable)
	FOnCancelEvent OnCancel;
};
