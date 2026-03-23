// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "Dialog/DialogAsset.h"

class FAssetTypeActions_DialogAsset : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_DialogAsset", "Dialog Asset"); }
	virtual FColor GetTypeColor() const override { return FColor(70, 160, 230); }
	virtual UClass* GetSupportedClass() const override { return UDialogAsset::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
};

