// Copyright 2022 Maximilien (Synock) Guislain

#include "DialogAndQuestPluginEditorModule.h"
#include "AssetTypeActions/AssetTypeActions_DialogAsset.h"
#include "AssetTypeActions/AssetTypeActions_QuestAsset.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "FDialogAndQuestPluginEditorModule"

void FDialogAndQuestPluginEditorModule::StartupModule()
{
	RegisterAssetTypeActions();
}

void FDialogAndQuestPluginEditorModule::ShutdownModule()
{
	UnregisterAssetTypeActions();
}

void FDialogAndQuestPluginEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	TSharedPtr<IAssetTypeActions> DialogAction = MakeShareable(new FAssetTypeActions_DialogAsset());
	AssetTools.RegisterAssetTypeActions(DialogAction.ToSharedRef());
	RegisteredAssetTypeActions.Add(DialogAction);

	TSharedPtr<IAssetTypeActions> QuestAction = MakeShareable(new FAssetTypeActions_QuestAsset());
	AssetTools.RegisterAssetTypeActions(QuestAction.ToSharedRef());
	RegisteredAssetTypeActions.Add(QuestAction);
}

void FDialogAndQuestPluginEditorModule::UnregisterAssetTypeActions()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& Action : RegisteredAssetTypeActions)
		{
			if (Action.IsValid())
				AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}
	RegisteredAssetTypeActions.Empty();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDialogAndQuestPluginEditorModule, DialogAndQuestPluginEditor)



