#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/DialogWindowInterface.h"
#include "DialogRepairWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseRepair);

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogRepairWidget : public UUserWidget
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
	UFUNCTION(BlueprintCallable, Category = "Dialog|Repair")
	void InitDialog(UObject* InputParentDialog);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Repair")
	void DoOnDisplay();
	virtual void DoOnDisplay_Implementation() {}

	UPROPERTY(BlueprintAssignable)
	FOnCloseRepair OnClose;
};
