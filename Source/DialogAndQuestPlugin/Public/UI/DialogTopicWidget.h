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
	void InitDialog(UObject* InputParentDialog);

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void UpdateTopicData();
};
