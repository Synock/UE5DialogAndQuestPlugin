// Copyright 2024 Maximilien (Synock) Guislain

#include "AssetTypeActions/AssetTypeActions_QuestAsset.h"
#include "Editor/QuestAssetEditor.h"
#include "Quest/QuestAsset.h"

void FQuestAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                              TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	for (UObject* Obj : InObjects)
	{
		if (UQuestAsset* Asset = Cast<UQuestAsset>(Obj))
			FQuestAssetEditor::OpenEditor(Mode, EditWithinLevelEditor, Asset);
	}
}

