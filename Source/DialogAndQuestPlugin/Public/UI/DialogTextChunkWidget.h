// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "DialogTextChunkWidget.generated.h"

USTRUCT(BlueprintType)
struct FDialogTextData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int64 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	FString TopicName;

	UPROPERTY(BlueprintReadWrite)
	FString TopicText;
};

UCLASS(BlueprintType)
class DIALOGANDQUESTPLUGIN_API UDialogTextChunkData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FDialogTextData Data;

	UPROPERTY(BlueprintReadWrite)
	class UDialogWindow* Parent = nullptr;
};


/**
 * 
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTextChunkWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void InitData(const FDialogTextData& ItemData);

	UPROPERTY(BlueprintReadOnly)
	int64 ItemID = 0;

	UPROPERTY(BlueprintReadOnly)
	FDialogTextData LocalData;

	UPROPERTY(BlueprintReadOnly)
	UDialogWindow* ParentDialog = nullptr;

	UPROPERTY(BlueprintReadWrite)
	URichTextBlock* TextBlock = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UTextBlock* TitleBock = nullptr;

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UFUNCTION(BlueprintCallable)
	FText GetTextData() const;

	UFUNCTION(BlueprintCallable)
	void SetTextData(const FText& NewTextData);
};
