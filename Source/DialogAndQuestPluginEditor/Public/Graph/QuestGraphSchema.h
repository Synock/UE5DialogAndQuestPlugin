// Copyright 2024 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "QuestGraphSchema.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Context-menu action: spawn a new step node.
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT()
struct FQuestSchemaAction_NewStep : public FEdGraphSchemaAction
{
	GENERATED_BODY()
	FQuestSchemaAction_NewStep() {}
	FQuestSchemaAction_NewStep(FText InCategory, FText InMenuDesc, FText InToolTip, int32 InGrouping)
		: FEdGraphSchemaAction(MoveTemp(InCategory), MoveTemp(InMenuDesc), MoveTemp(InToolTip), InGrouping) {}

	virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, FVector2D Location, bool bSelectNewNode) override;
};

// ─────────────────────────────────────────────────────────────────────────────
// Schema
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class UQuestGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()
public:
	/** Shared pin category for all Quest flow pins. */
	static const FName PC_Flow;

	// ── UEdGraphSchema interface ───────────────────────────────────────────
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual void GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotification) const override;
	virtual bool ShouldAlwaysPurgeOnModification() const override { return true; }

	/** Returns the next unused QuestSubID in the graph. */
	static int32 AllocateSubID(const UEdGraph* Graph);
};



