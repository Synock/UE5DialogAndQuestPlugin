#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/DialogWindowInterface.h"
#include "UI/DialogTextChunkWidget.h"
#include "DialogTopicButtonWidget.generated.h"

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTopicButtonWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	/** Owning dialog window (implements IDialogWindowInterface). */
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UObject> ParentDialogObject = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FDialogTextData LocalData;

	/** The clickable button wrapping the topic. Name must match in the Blueprint layout. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog")
	TObjectPtr<class UButton> TopicButton = nullptr;

	/** Text label showing the topic name. Name must match in the Blueprint layout. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog")
	TObjectPtr<class UTextBlock> TopicLabel = nullptr;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnTopicButtonClicked();

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

public:
	/** Called to populate this entry from data. Override in Blueprint for custom styling. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog")
	void InitData(const FDialogTextData& ItemData);
	virtual void InitData_Implementation(const FDialogTextData& ItemData);

	UFUNCTION(BlueprintCallable)
	void InitParent(UObject* Parent);
};
