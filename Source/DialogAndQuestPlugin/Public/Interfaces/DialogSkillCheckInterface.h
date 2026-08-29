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
	/// Implementations should check ALL persistent storage locations (active inventory bags
	/// AND bank) so that AbsentItems conditions and RequiredItems conditions behave
	/// consistently regardless of where the player has stashed the item.
	/// @param RequiredItems Array of item IDs that must be found in inventory or bank.
	/// @param CheckingActor The actor being checked.
	/// @return True if every item in RequiredItems is present in inventory or bank.
	virtual bool HasRequiredItems(const TArray<int32>& RequiredItems, const AActor* CheckingActor) const
	{
		return RequiredItems.IsEmpty(); // Default: no items required = pass
	}

	/// Check whether the actor effectively owns any item in the supplied list.
	/// This is used by AbsentItems guards, whose semantics differ from RequiredItems:
	/// finding any excluded item must fail the condition. Games may include protected
	/// pending grants here so a recovery topic cannot duplicate an item awaiting delivery.
	/// The default implementation remains backwards-compatible by checking each item
	/// individually through HasRequiredItems().
	virtual bool HasAnyOwnedItems(const TArray<int32>& ItemIDs, const AActor* CheckingActor) const
	{
		for (const int32 ItemID : ItemIDs)
		{
			if (HasRequiredItems({ItemID}, CheckingActor))
				return true;
		}
		return false;
	}
};
