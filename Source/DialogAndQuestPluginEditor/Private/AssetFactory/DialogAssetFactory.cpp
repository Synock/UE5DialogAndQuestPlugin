// Copyright 2022 Maximilien (Synock) Guislain

#include "AssetFactory/DialogAssetFactory.h"
#include "Dialog/DialogAsset.h"

UDialogAssetFactory::UDialogAssetFactory()
{
	SupportedClass = UDialogAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDialogAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
                                                EObjectFlags Flags, UObject* Context,
                                                FFeedbackContext* Warn)
{
	return NewObject<UDialogAsset>(InParent, Class, Name, Flags);
}

FText UDialogAssetFactory::GetDisplayName() const
{
	return NSLOCTEXT("DialogAndQuest", "DialogAssetFactory", "Dialog Asset");
}

