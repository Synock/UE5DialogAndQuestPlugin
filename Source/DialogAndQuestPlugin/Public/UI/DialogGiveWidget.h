#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/DialogWindowInterface.h"
#include "DialogGiveWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGiveEvent);

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogGiveWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UObject> ParentDialogObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<const UDialogComponent> DialogComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> GiveButton = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> CancelButton = nullptr;

	virtual void NativeConstruct() override;

	UFUNCTION() void OnGiveButtonClicked();
	UFUNCTION() void OnCancelButtonClicked();

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void InitDialog(UObject* InputParentDialog);

	UPROPERTY(BlueprintAssignable) FOnGiveEvent   OnGive;
	UPROPERTY(BlueprintAssignable) FOnCancelEvent OnCancel;
};
