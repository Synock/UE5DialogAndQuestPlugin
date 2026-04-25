#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Interfaces/DialogWindowInterface.h"
#include "UI/DialogTextChunkWidget.h"
#include "DialogTextWidget.generated.h"

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTextWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UObject> ParentDialogObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<const UDialogComponent> DialogComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog|Widgets")
	TObjectPtr<UListView> ListViewWidget = nullptr;

	UFUNCTION(BlueprintCallable)
	FString ProcessText(const FString& InputString) const;

	void AddTopicData(const FDialogTextData& DialogTopic);

public:
	UFUNCTION(BlueprintCallable)
	virtual void ClearList();

	UFUNCTION(BlueprintCallable)
	virtual void AddEmptyTopicData(const FString& DialogText);

	UFUNCTION(BlueprintCallable)
	virtual void InitDialog(UObject* InputParentDialog);

	UFUNCTION(BlueprintCallable)
	virtual void AddTopicText(int64 TopicID);

	/** Reprocess already-displayed text to resolve newly-available hyperlinks. */
	UFUNCTION(BlueprintCallable)
	void ReprocessTopicLinks();
};
