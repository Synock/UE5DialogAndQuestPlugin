// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "Quest/QuestAsset.h"

class FAssetTypeActions_QuestAsset : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_QuestAsset", "Quest Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(230, 180, 50); }
	virtual UClass* GetSupportedClass() const override { return UQuestAsset::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
};

