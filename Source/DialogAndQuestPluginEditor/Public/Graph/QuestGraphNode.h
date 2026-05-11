// Copyright 2024 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "Quest/QuestData.h"
#include "QuestGraphNode.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Entry node — singleton, cannot be deleted, has one "Start" output pin only.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(MinimalAPI)
class UQuestGraphNode_Entry : public UEdGraphNode
{
	GENERATED_BODY()
public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual bool CanUserDeleteNode() const override { return false; }
	virtual bool CanDuplicateNode() const override  { return false; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Step node — one per FQuestStep entry in UQuestAsset::Steps.
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(MinimalAPI)
class UQuestGraphNode_Step : public UEdGraphNode
{
	GENERATED_BODY()
public:
	// ── Editable step content (primary Details-panel surface) ─────────────
	/**
	 * Full step data — edit title, description, items, rewards, voiceover here.
	 * Fields managed by the graph (do not edit manually):
	 *   QuestID / QuestSubID — overwritten by the graph on Compile
	 *   StepType   — synced from/to StepType below; editable here too
	 *   NextStepIDs — rebuilt from output-pin connections on Compile
	 */
	UPROPERTY(EditAnywhere, Category = "Step")
	FQuestStep StepData;

	// ── Graph-topology fields (set by graph, shown read-only) ─────────────
	/** Canonical SubID — links this node to the backing FQuestStep. */
	UPROPERTY(VisibleAnywhere, Category = "Step|Graph")
	int32 StepSubID = 0;

	/**
	 * Controls output pin layout. Synced to/from StepData.StepType.
	 * Prefer the right-click context menu; editing StepData.StepType also works.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Step|Graph")
	EQuestStepType StepType = EQuestStepType::Linear;

	/** Number of output pins for Branch / Parallel nodes. */
	UPROPERTY(EditAnywhere, Category = "Step|Graph", meta = (ClampMin = "2", ClampMax = "8"))
	int32 BranchCount = 2;

	// ── UEdGraphNode interface ─────────────────────────────────────────────
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	virtual void NodeConnectionListChanged() override;

#if WITH_EDITOR
	/**
	 * Reacts to Details-panel edits:
	 *   StepData.StepType → syncs StepType, rebuilds output pins
	 *   BranchCount       → rebuilds output pins
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ── Helpers ───────────────────────────────────────────────────────────
	/** Single input pin. */
	UEdGraphPin* GetInputPin() const;
	/** All output pins (may be > 1 for Branch / Parallel). */
	TArray<UEdGraphPin*> GetOutputPins() const;

	/**
	 * Rebuild output pins to match StepType and BranchCount.
	 * Existing wire connections are preserved where pin names match.
	 * @param bNotifyGraph  Pass false during bulk graph construction to avoid
	 *                      per-node redraws; caller must call NotifyGraphChanged() itself.
	 */
	void RebuildOutputPins(bool bNotifyGraph = true);

	/**
	 * Immediately copy this node's StepData into the matching FQuestStep in
	 * the owning UQuestAsset::Steps[] array, preserving graph-managed topology
	 * fields (QuestID, QuestSubID, StepType, NextStepIDs).
	 *
	 * Called from PostEditChangeProperty and NodeConnectionListChanged so that
	 * content edits (title, description, items, rewards…) are visible in the
	 * asset's array view without requiring an explicit Compile.
	 */
	void SyncStepToAsset();

	/** Pin category shared by all QuestGraph pins. */
	static const FName PinCategory;

	/** Pin names */
	static const FName PinName_In;
	static const FName PinName_Next;
};
