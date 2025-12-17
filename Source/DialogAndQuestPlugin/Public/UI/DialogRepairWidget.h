#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogRepairWidget.generated.h"

/**
 * Dialog repair widget for handling item repair functionality
 */
class UDialogComponent;
class UDialogWindow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseRepair);

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogRepairWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// UUserWidget interface
	virtual void NativeConstruct() override;

public:
	// Called when the widget is displayed
	UPROPERTY(BlueprintReadOnly)
	UDialogWindow* ParentDialog = nullptr;

	UPROPERTY(BlueprintReadOnly)
	const UDialogComponent* DialogComponent = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Dialog|Repair")
	void DoOnDisplay();

	UFUNCTION(BlueprintCallable, Category = "Dialog|Repair")
	void InitDialog(UDialogWindow* InputParentDialog);

	UPROPERTY(BlueprintAssignable)
	FOnCloseRepair OnClose;
};



