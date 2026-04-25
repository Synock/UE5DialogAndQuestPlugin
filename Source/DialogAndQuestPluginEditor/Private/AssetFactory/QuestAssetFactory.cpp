// Copyright 2022 Maximilien (Synock) Guislain

#include "AssetFactory/QuestAssetFactory.h"
#include "Quest/QuestAsset.h"

UQuestAssetFactory::UQuestAssetFactory()
{
	SupportedClass = UQuestAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UQuestAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
                                               EObjectFlags Flags, UObject* Context,
                                               FFeedbackContext* Warn)
{
	return NewObject<UQuestAsset>(InParent, Class, Name, Flags);
}

FText UQuestAssetFactory::GetDisplayName() const
{
	return NSLOCTEXT("DialogAndQuest", "QuestAssetFactory", "Quest Asset");
}

