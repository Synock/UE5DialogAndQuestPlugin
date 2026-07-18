
#include "Dialog/DialogData.h"

#include "Interfaces/DialogInterface.h"
#include "Interfaces/DialogSkillCheckInterface.h"
#include "Interfaces/QuestBearerInterface.h"
#include "Misc/DialogAndQuestPluginHelper.h"

bool FDialogTopicCondition::VerifyCondition(const AActor* DialogActor, const APlayerController* Controller,
	bool bCheckRelation, bool bRequireQuestNotKnown) const
{
	if (!Controller)
		return false;

	// Relation check
	bool bRelationOK = true;
	if (bCheckRelation)
	{
		if (const IDialogInterface* DialogInterfaceActor = Cast<IDialogInterface>(DialogActor))
		{
			if (Controller->GetPawn())
				bRelationOK = DialogInterfaceActor->GetRelation(Controller->GetPawn()) >= MinimumRelation;
			else
				bRelationOK = false;
		}
	}

	// Skill check (game-agnostic via interface)
	if (SkillCheckTag.IsValid())
	{
		if (const IDialogSkillCheckInterface* SkillCheck = Cast<IDialogSkillCheckInterface>(Controller))
		{
			if (SkillCheck->EvaluateSkillCheck(SkillCheckTag, Controller->GetPawn()) < MinimumSkillValue)
				return false;
		}
		else
		{
			// Controller doesn't implement skill checks — fail the condition
			return false;
		}
	}

	// Required items check (game-agnostic via interface)
	if (!RequiredItems.IsEmpty())
	{
		if (const IDialogSkillCheckInterface* SkillCheck = Cast<IDialogSkillCheckInterface>(Controller))
		{
			if (!SkillCheck->HasRequiredItems(RequiredItems, Controller->GetPawn()))
				return false;
		}
		else
		{
			return false;
		}
	}

	// Absent items check — topic only shows when the player does NOT possess these items.
	// Relies on the same HasRequiredItems path (which covers inventory + bank).
	if (!AbsentItems.IsEmpty())
	{
		if (const IDialogSkillCheckInterface* SkillCheck = Cast<IDialogSkillCheckInterface>(Controller))
		{
			// If the player HAS any of the absent items, hide this topic.
			if (SkillCheck->HasRequiredItems(AbsentItems, Controller->GetPawn()))
				return false;
		}
		else
		{
			// Controller has no item-check support — cannot confirm absence, so hide.
			return false;
		}
	}

	// Quest condition
	if (bRequireQuestNotKnown && GetQuestID() == 0)
		return false;

	if (GetQuestID() != 0)
	{
		const IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(Controller);
		if (!QuestBearer)
			return false;

		if (bRequireQuestNotKnown)
			return !QuestBearer->IsQuestKnown(GetQuestID()) && bRelationOK;

		const bool bHasStateFilter = RequiredQuestState != EQuestState::Unknown;
		const bool bHasStepFilter  = MinimumStepID != 0;

		// If the quest is not known at all:
		// - A state filter can only pass for Unknown (which means "no filter")
		// - A step filter always fails (CanDisplay returns false for unknown quests)
		if (!QuestBearer->IsQuestKnown(GetQuestID()))
			return false;

		// State-based check
		if (bHasStateFilter)
		{
			const FQuestProgressData& Progress = QuestBearer->GetKnownQuest(GetQuestID());
			if (Progress.State != RequiredQuestState)
				return false;
		}

		// Step-based check (combined with state when both are set)
		if (bHasStepFilter)
		{
			if (!QuestBearer->CanDisplay(GetQuestID(), MinimumStepID, StepCondition))
				return false;
		}

		// If neither filter was set (quest != null but no state/step), just require quest to be known
		return bRelationOK;
	}

	return bRelationOK;
}

bool FConditionalGreeting::Matches(const AActor* DialogActor, const APlayerController* Controller) const
{
	return !Text.IsEmpty() && Condition.VerifyCondition(DialogActor, Controller, bCheckRelation, bRequireQuestNotKnown);
}

TSoftObjectPtr<USoundBase> FConditionalGreeting::GetVoiceover() const
{
	return !VoiceoverPath.IsEmpty()
		? TSoftObjectPtr<USoundBase>(FSoftObjectPath(VoiceoverPath))
		: Voiceover;
}

const FConditionalGreeting* FindFirstMatchingConditionalGreeting(
	const TArray<FConditionalGreeting>& Greetings, const AActor* DialogActor, const APlayerController* Controller)
{
	for (const FConditionalGreeting& Greeting : Greetings)
	{
		if (Greeting.Matches(DialogActor, Controller))
			return &Greeting;
	}

	return nullptr;
}
