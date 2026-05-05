// Copyright 2023 Maximilien (Synock) Guislain

#include "Dialog/DialogAsset.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult UDialogAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

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

