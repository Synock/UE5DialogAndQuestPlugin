// Copyright 2024 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "QuestGraph.generated.h"

class UQuestAsset;
class UQuestGraphNode_Entry;
class UQuestGraphNode_Step;

/**
 * Editor-only graph for UQuestAsset.
 *
 * Serialised as WITH_EDITORONLY_DATA on the asset and stripped at cook.
 * The canonical data is always UQuestAsset::Steps[] — the graph is a view
 * onto that data plus layout/wiring.
 *
 * Workflow:
 *   BuildFromAsset()  — populate graph from Steps[] on asset open / undo
 *   CompileToAsset()  — write flow topology back into Steps[] on save
 */
UCLASS()
class UQuestGraph : public UEdGraph
{
	GENERATED_BODY()
public:
	/**
	 * Populate nodes from Asset->Steps.
	 * First call: auto-arranges nodes in a left-to-right topological layout.
	 * Subsequent calls: preserves existing NodePosX/Y.
	 */
	void BuildFromAsset(UQuestAsset* Asset);

	/**
	 * Write flow topology (StepType, NextStepIDs, Steps[] order) back to Asset.
	 * Returns false on validation errors (fills OutErrors — shown in editor).
	 * Never discards unknown FQuestStep data (descriptions, items, etc.).
	 */
	bool CompileToAsset(UQuestAsset* Asset, TArray<FString>& OutErrors);

	/** Find the singleton Entry node, or nullptr if missing. */
	UQuestGraphNode_Entry* GetEntryNode() const;

	/** Find the step node for a given SubID, or nullptr. */
	UQuestGraphNode_Step* FindStepNode(int32 SubID) const;

private:
	/**
	 * Arrange all nodes in a left-to-right BFS layout starting from the entry.
	 * Called once on first BuildFromAsset when no positions have been saved.
	 */
	void AutoArrangeNodes();

	/** True once layout has been saved so we don't clobber the user's positions. */
	UPROPERTY()
	bool bHasSavedLayout = false;

	/**
	 * Weak back-reference to the owning asset.
	 * Set by BuildFromAsset; used by nodes to sync content edits directly
	 * without waiting for an explicit Compile.
	 */
	UPROPERTY()
	TObjectPtr<UQuestAsset> OwnerAsset;

public:
	/** Public accessor so graph nodes can reach the asset for live sync. */
	UQuestAsset* GetOwnerAsset() const { return OwnerAsset.Get(); }
};
