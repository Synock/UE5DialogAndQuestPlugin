// Copyright 2022 Maximilien (Synock) Guislain

#include "DialogAndQuestPluginEditorModule.h"
#include "AssetTypeActions/AssetTypeActions_DialogAsset.h"
#include "AssetTypeActions/AssetTypeActions_QuestAsset.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "PropertyEditorModule.h"
#include "Quest/QuestAsset.h"
#include "Dialog/DialogAsset.h"
#include "DetailCustomization/QuestAssetDetailCustomization.h"
#include "DetailCustomization/DialogAssetDetailCustomization.h"
// Graph node classes must be linked so their UHT-generated code is registered
#include "Graph/QuestGraph.h"
#include "Graph/QuestGraphNode.h"
#include "Graph/QuestGraphSchema.h"

#define LOCTEXT_NAMESPACE "FDialogAndQuestPluginEditorModule"

void FDialogAndQuestPluginEditorModule::StartupModule()
{
	RegisterAssetTypeActions();
	RegisterDetailCustomizations();
}

void FDialogAndQuestPluginEditorModule::ShutdownModule()
{
	UnregisterDetailCustomizations();
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

void FDialogAndQuestPluginEditorModule::RegisterDetailCustomizations()
{
	FPropertyEditorModule& PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout(
		UQuestAsset::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FQuestAssetDetailCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UDialogAsset::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FDialogAssetDetailCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FDialogAndQuestPluginEditorModule::UnregisterDetailCustomizations()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule =
			FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UQuestAsset::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UDialogAsset::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDialogAndQuestPluginEditorModule, DialogAndQuestPluginEditor)

