#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/DialogWindowInterface.h"
#include "DialogFooterWidget.generated.h"

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogFooterWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UObject> ParentDialogObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<const UDialogComponent> DialogComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog|Buttons")
	TObjectPtr<class UButton> CloseButton = nullptr;

	virtual void NativeConstruct() override;

	UFUNCTION() void OnCloseButtonClicked();

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void InitDialog(UObject* InputParentDialog);
};
