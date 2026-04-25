#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Dialog/DialogData.h"
#include "UI/DialogTextChunkWidget.h"
#include "DialogWindowInterface.generated.h"

class UDialogComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UDialogWindowInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for any UUserWidget subclass that acts as the dialog window.
 *
 * ## Design (mirrors IInventoryBookWidgetInterface for consistency)
 *
 * ### Logic methods (InitDialogWindow, DisplayDialogTopic, DisplayDialogTopicFromString,
 *     DisplayPlainString, RefreshDialogOptions, CloseWindow)
 * Full C++ default implementations are provided in DialogWindowInterface.cpp.
 * They handle consequence firing, voiceover dispatch, quest-relation progression,
 * and topic condition filtering before delegating to the appropriate visual hook.
 * A custom implementing class (e.g. UFinalDialogWindow) generally needs to override
 * only the visual hooks — not these.
 *
 * ### Visual hooks (OnDialogOpened, OnTopicTextReady, OnTopicListUpdated,
 *     OnDialogClosed, Display*Widget tab navigations)
 * Empty C++ defaults. Override these in the Blueprint subclass to control exactly
 * how the UI responds to dialog state changes.
 *
 * ### Data accessors (GetDialogComponent, GetDialogActor)
 * Must be overridden in C++: the implementing class stores the UDialogComponent and
 * dialog NPC actor received during InitDialogWindow and returns them here.
 *
 * ## Backward compatibility
 * UDialogWindow remains a complete concrete implementation. All its sub-widgets
 * (UDialogHeaderWidget, UDialogTopicWidget, etc.) now reference this interface
 * instead of UDialogWindow directly, making the plugin fully game-agnostic.
 */
class DIALOGANDQUESTPLUGIN_API IDialogWindowInterface
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------------------------
	// Data accessors — no useful default; override in C++ to return stored refs.
	// -------------------------------------------------------------------------

	/** Returns the UDialogComponent belonging to the NPC currently in dialog. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window")
	UDialogComponent* GetDialogComponent() const;
	virtual UDialogComponent* GetDialogComponent_Implementation() const { return nullptr; }

	/** Returns the NPC actor currently in dialog. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window")
	AActor* GetDialogActor() const;
	virtual AActor* GetDialogActor_Implementation() const { return nullptr; }

	// -------------------------------------------------------------------------
	// Lifecycle — C++ defaults handle all logic; visual hooks fire from inside.
	// -------------------------------------------------------------------------

	/** Initialize the dialog window with a given NPC. Fires OnDialogOpened when done. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window")
	void InitDialogWindow(UDialogComponent* InputDialogComponent, AActor* ActorDialog);
	virtual void InitDialogWindow_Implementation(UDialogComponent* InputDialogComponent, AActor* ActorDialog);

	/** Close the dialog. Fires OnDialogClosed. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window")
	void CloseWindow();
	virtual void CloseWindow_Implementation();

	/** Re-evaluate topic visibility and fire OnTopicListUpdated with the filtered set. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window")
	void RefreshDialogOptions();
	virtual void RefreshDialogOptions_Implementation();

	// -------------------------------------------------------------------------
	// Topic display — C++ defaults handle consequences, voiceover, and the rest.
	// -------------------------------------------------------------------------

	/** Display a topic by ID. Handles consequences, voiceover, quest relations. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window")
	void DisplayDialogTopic(int64 ID);
	virtual void DisplayDialogTopic_Implementation(int64 ID);

	/** Look up a topic keyword string, then call DisplayDialogTopic. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window")
	void DisplayDialogTopicFromString(const FString& ID);
	virtual void DisplayDialogTopicFromString_Implementation(const FString& ID);

	/** Show an arbitrary plain-text string in the response area. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window")
	void DisplayPlainString(const FString& PlainString);
	virtual void DisplayPlainString_Implementation(const FString& PlainString);

	// -------------------------------------------------------------------------
	// Tab navigation — empty C++ defaults; Blueprint overrides to control layout.
	// -------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Tabs")
	void DisplayMainDialogWidget();
	virtual void DisplayMainDialogWidget_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Tabs")
	void DisplayTradeWidget();
	virtual void DisplayTradeWidget_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Tabs")
	void DisplayGiveWidget();
	virtual void DisplayGiveWidget_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Tabs")
	void DisplayTrainDialogWidget();
	virtual void DisplayTrainDialogWidget_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Tabs")
	void DisplayBankDialogWidget();
	virtual void DisplayBankDialogWidget_Implementation() {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Tabs")
	void DisplayRepairDialogWidget();
	virtual void DisplayRepairDialogWidget_Implementation() {}

	// -------------------------------------------------------------------------
	// Visual hooks — empty C++ defaults; implement in Blueprint for custom UI.
	// -------------------------------------------------------------------------

	/**
	 * Called after InitDialogWindow has stored the component/actor.
	 * @param Greeting        The NPC's greeting (good or bad, relation-gated).
	 * @param RelationValue   Raw relation value [0..1].
	 * @param RelationString  Human-readable label ("Amiable", "Threatening", …).
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Events")
	void OnDialogOpened(const FText& Greeting, float RelationValue, const FString& RelationString);
	virtual void OnDialogOpened_Implementation(const FText& Greeting, float RelationValue, const FString& RelationString) {}

	/**
	 * Called when topic-response text (or a plain string) is ready to display.
	 * TextData.TopicText already has hyperlinks processed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Events")
	void OnTopicTextReady(const FDialogTextData& TextData);
	virtual void OnTopicTextReady_Implementation(const FDialogTextData& TextData) {}

	/**
	 * Called each time the visible topic list changes (after init and after each click).
	 * VisibleTopics is pre-filtered: only topics whose VerifyCondition returned true.
	 * Rebuild the topic button list from this array.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Events")
	void OnTopicListUpdated(const TArray<FDialogTopicStruct>& VisibleTopics);
	virtual void OnTopicListUpdated_Implementation(const TArray<FDialogTopicStruct>& VisibleTopics) {}

	/** Called just before the dialog window closes. Hide/animate the widget here. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialog|Window|Events")
	void OnDialogClosed();
	virtual void OnDialogClosed_Implementation() {}
};

