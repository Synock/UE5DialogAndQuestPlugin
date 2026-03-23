#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "DialogSkillCheckInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDialogSkillCheckInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for evaluating skill checks in dialog conditions.
 * The game implements this on the PlayerController or a subsystem.
 * The plugin calls EvaluateSkillCheck() during FDialogTopicCondition::VerifyCondition().
 * This keeps the plugin game-agnostic — it never knows about specific skills.
 */
class DIALOGANDQUESTPLUGIN_API IDialogSkillCheckInterface
{
	GENERATED_BODY()

public:
	/// Evaluate a skill check for the given tag.
	/// @param SkillTag GameplayTag identifying the skill (e.g. "Skill.Persuasion").
	/// @param CheckingActor The actor being checked (usually the player pawn).
	/// @return The actor's current value for this skill. Compared against MinimumSkillValue in the condition.
	virtual float EvaluateSkillCheck(const FGameplayTag& SkillTag, const AActor* CheckingActor) const = 0;

	/// Check if the given actor possesses all the required items.
	/// @param RequiredItems Array of item IDs that must be in the actor's inventory.
	/// @param CheckingActor The actor being checked.
	/// @return True if all required items are present.
	virtual bool HasRequiredItems(const TArray<int32>& RequiredItems, const AActor* CheckingActor) const
	{
		return RequiredItems.IsEmpty(); // Default: no items required = pass
	}
};

