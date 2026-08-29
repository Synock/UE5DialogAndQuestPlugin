#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Interfaces/DialogWindowInterface.h"
#include "UI/DialogTextChunkWidget.h"
#include "DialogTopicWidget.generated.h"

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTopicWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<UObject> ParentDialogObject = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialog")
	TObjectPtr<const UDialogComponent> DialogComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog|Widgets")
	TObjectPtr<UListView> TopicListView = nullptr;

	void AddTopicData(const FDialogTextData& DialogTopic);
	void ClearList();

public:
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	virtual void InitDialog(UObject* InputParentDialog);

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void UpdateTopicData();

#if WITH_AUTOMATION_WORKER || (WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS)
	int32 GetVisibleTopicCountForTests() const { return TopicListView ? TopicListView->GetNumItems() : 0; }
	void SetConditionControllerForTests(APlayerController* Controller) { ConditionControllerForTests = Controller; }

private:
	TWeakObjectPtr<APlayerController> ConditionControllerForTests;
#endif
};
