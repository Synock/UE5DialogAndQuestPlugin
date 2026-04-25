#pragma once

#include "CoreMinimal.h"
#include "Dialog/DialogData.h"
#include "UObject/Interface.h"
#include "DialogConsequenceInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDialogConsequenceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for handling dialog consequences. The game's PlayerController should implement this
 * to process faction changes, quest state transitions, and quest mentions triggered by dialog.
 */
class DIALOGANDQUESTPLUGIN_API IDialogConsequenceInterface
{
	GENERATED_BODY()

public:
	/// Called when a dialog topic is clicked.
	/// @param TopicID  The Id of the FDialogTopicStruct that was clicked — NOT the consequence itself.
	///                 The server re-fetches and re-validates the topic before processing any consequence.
	/// @param DialogActor The NPC the player is talking to.
	virtual void HandleDialogConsequence(int64 TopicID, AActor* DialogActor) = 0;

	/// Called to apply a faction relation delta.
	/// @param DialogActor The NPC whose faction is adjusted.
	/// @param Delta The relation change amount.
	virtual void ApplyFactionDelta(AActor* DialogActor, float Delta) {}

	/// Called to "mention" a quest (transition it to the Mentioned state).
	/// @param QuestID The quest to mention.
	virtual void MentionQuest(int64 QuestID) {}

	/// Called to transition a quest to a new state via dialog.
	/// @param QuestID The quest to transition.
	/// @param NewState The target state.
	/// @param DialogActor The NPC triggering the transition.
	virtual void TransitionQuestState(int64 QuestID, EQuestState NewState, AActor* DialogActor) {}
};

