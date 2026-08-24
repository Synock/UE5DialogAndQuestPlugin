// Copyright 2023 Maximilien (Synock) Guislain

#include "Dialog/DialogAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult UDialogAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	TSet<FName> ConditionalGreetingIds;
	for (const FConditionalGreeting& Greeting : ConditionalGreetings)
	{
		if (Greeting.Id.IsNone())
		{
			Context.AddError(FText::FromString(TEXT("Conditional greetings require a non-empty Id for dialogue-pipeline export.")));
			Result = EDataValidationResult::Invalid;
		}
		else if (ConditionalGreetingIds.Contains(Greeting.Id))
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("Conditional greeting Id '%s' is duplicated. Ids must be unique within a dialog asset."), *Greeting.Id.ToString())));
			Result = EDataValidationResult::Invalid;
		}
		else
		{
			ConditionalGreetingIds.Add(Greeting.Id);
		}

		if (Greeting.Text.IsEmpty())
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("Conditional greeting '%s' has no text."), *Greeting.Id.ToString())));
			Result = EDataValidationResult::Invalid;
		}

		if (Greeting.bRequireQuestNotKnown && Greeting.Condition.GetQuestID() == 0)
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("Conditional greeting '%s' requires a Quest when 'Require Quest Not Known' is enabled."), *Greeting.Id.ToString())));
			Result = EDataValidationResult::Invalid;
		}

		if (Greeting.Condition.GetQuestID() != 0 && Greeting.Condition.MinimumStepID == 0 &&
			!Greeting.Condition.bUseStepFilter)
		{
			Context.AddWarning(FText::FromString(FString::Printf(
				TEXT("Conditional greeting '%s' does not filter quest step 0; it matches any known state of its quest. "
					 "Enable 'Use Step Filter' to require exact step 0."), *Greeting.Id.ToString())));
		}
	}

	TSet<const UDialogAsset*> Visited;
	TSet<const UDialogAsset*> Stack;

	if (HasSharedAssetCycle(this, Visited, Stack))
	{
		Context.AddError(FText::FromString(FString::Printf(
			TEXT("DialogAsset '%s' (%s) has a circular reference in SharedTopicAssets. "
			     "Remove the self/circular entry to fix this."),
			*AssetName, *GetPathName())));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}

bool UDialogAsset::HasSharedAssetCycle(const UDialogAsset* Node, TSet<const UDialogAsset*>& Visited, TSet<const UDialogAsset*>& Stack) const
{
	if (!Node)
		return false;

	if (Stack.Contains(Node))
		return true; // Back edge — cycle found

	if (Visited.Contains(Node))
		return false; // Already fully explored, no cycle through this node

	Visited.Add(Node);
	Stack.Add(Node);

	for (const TSoftObjectPtr<UDialogAsset>& SharedRef : Node->SharedTopicAssets)
	{
		const UDialogAsset* Shared = SharedRef.Get();
		if (!Shared)
			Shared = SharedRef.LoadSynchronous();

		if (Shared && HasSharedAssetCycle(Shared, Visited, Stack))
			return true;
	}

	Stack.Remove(Node);
	return false;
}

#endif // WITH_EDITOR
