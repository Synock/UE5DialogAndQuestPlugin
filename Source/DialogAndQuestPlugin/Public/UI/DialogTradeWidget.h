#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/DialogWindowInterface.h"
#include "DialogTradeWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseTrade);

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTradeWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UObject> ParentDialogObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<const UDialogComponent> DialogComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> CloseButton = nullptr;

	virtual void NativeConstruct() override;

	UFUNCTION() void OnCloseButtonClicked();

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void InitDialog(UObject* InputParentDialog);

	UPROPERTY(BlueprintAssignable) FOnCloseTrade OnClose;
};
