// Copyright 2022 Maximilien (Synock) Guislain

using UnrealBuildTool;

public class DialogAndQuestPluginEditor : ModuleRules
{
	public DialogAndQuestPluginEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"DialogAndQuestPlugin",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"AssetTools",
				"EditorStyle",
				"PropertyEditor",
				"InputCore",
				"GameplayTags",
				// Graph editor
				"GraphEditor",
				"ToolMenus",
				"EditorFramework",
				"ApplicationCore",
			}
		);
	}
}

