// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UQuestAsset;

/**
 * Detail customization for UQuestAsset.
 *
 * Adds a "Quest Tools" section at the top of the details panel with:
 *  - "Assign Next Quest ID": assigns the next project-wide QuestID and syncs every step.
 *  - "Sync Step QuestIDs"  : propagates the asset-level QuestID to all FQuestStep::QuestID fields.
 *  - "Sequence SubIDs"     : renumbers QuestSubID 0..N-1 in array order.
 *  - Inline validation     : warns on mismatched QuestIDs and duplicate SubIDs, refreshed after each button.
 */
class FQuestAssetDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply OnAssignNextQuestID();
	FReply OnSyncStepQuestIDs();
	FReply OnAutoSequenceSubIDs();

	TWeakObjectPtr<UQuestAsset> QuestAssetPtr;
	IDetailLayoutBuilder* CachedDetailBuilder = nullptr;
};
