// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FDialogAndQuestPluginEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterAssetTypeActions();
	void UnregisterAssetTypeActions();

	void RegisterDetailCustomizations();
	void UnregisterDetailCustomizations();

	TArray<TSharedPtr<class IAssetTypeActions>> RegisteredAssetTypeActions;
};
