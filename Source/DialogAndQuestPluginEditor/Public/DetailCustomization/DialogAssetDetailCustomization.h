// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UDialogAsset;

/**
 * Detail customization for UDialogAsset.
 *
 * Adds a "Dialog Tools" section at the top of the details panel with:
 *  - "Assign Next Topic IDs" : fills any Topic.Id == 0 with auto-incremented IDs (max+1, max+2...).
 *  - Inline validation       : warns on duplicate topic IDs and unset (0) IDs, refreshed after the button.
 */
class FDialogAssetDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply OnAssignNextTopicIDs();

	TWeakObjectPtr<UDialogAsset> DialogAssetPtr;
	IDetailLayoutBuilder* CachedDetailBuilder = nullptr;
};

