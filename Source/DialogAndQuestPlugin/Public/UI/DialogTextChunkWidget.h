#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Sound/SoundBase.h"
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

	/// Soft reference to the voiceover sound for this line.
	/// Valid when Id != 0 (i.e., this chunk came from a topic, not a plain string).
	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> VoiceoverCue;

	/// Duration hint for subtitle display in seconds.
	/// 0 = use the sound asset's own duration (or a fallback default).
	UPROPERTY(BlueprintReadWrite)
	float VoiceoverDuration = 0.f;
};

UCLASS(BlueprintType)
class DIALOGANDQUESTPLUGIN_API UDialogTextChunkData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FDialogTextData Data;

	/** The dialog window that owns this chunk. Typed as UObject so the plugin
	 *  is agnostic to the concrete dialog window class (UDialogWindow or
	 *  UFinalDialogWindow both implement IDialogWindowInterface). */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> Parent = nullptr;
};


/**
 * List entry widget for a single dialog text chunk (NPC response line).
 * TextBlock (required) and TitleBlock (optional header/speaker label) are
 * bound in C++ — the Blueprint only needs to provide the layout.
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogTextChunkWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Dialog")
	TObjectPtr<URichTextBlock> TextBlock = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Dialog")
	TObjectPtr<UTextBlock> TitleBlock = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int64 ItemID = 0;

	UPROPERTY(BlueprintReadOnly)
	FDialogTextData LocalData;

	/** Owning dialog window (implements IDialogWindowInterface). */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> ParentDialogObject = nullptr;

	/** Called after TextBlock/TitleBlock are populated. Override in Blueprint for custom styling. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog")
	void InitData(const FDialogTextData& ItemData);
	virtual void InitData_Implementation(const FDialogTextData& ItemData);

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UFUNCTION(BlueprintCallable)
	FText GetTextData() const;

	UFUNCTION(BlueprintCallable)
	void SetTextData(const FText& NewTextData);
};
