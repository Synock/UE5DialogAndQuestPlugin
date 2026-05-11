// Copyright 2024 Maximilien (Synock) Guislain

#include "Graph/QuestGraphSchema.h"

#include "Graph/QuestGraph.h"
#include "Graph/QuestGraphNode.h"
#include "Quest/QuestAsset.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "Framework/Commands/GenericCommands.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"
#include "GraphEditorActions.h"

#define LOCTEXT_NAMESPACE "QuestGraphSchema"

const FName UQuestGraphSchema::PC_Flow = TEXT("QuestFlow");

// ─────────────────────────────────────────────────────────────────────────────
// FQuestSchemaAction_NewStep
// ─────────────────────────────────────────────────────────────────────────────

UEdGraphNode* FQuestSchemaAction_NewStep::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
                                                         FVector2D Location, bool bSelectNewNode)
{
	const FScopedTransaction Transaction(LOCTEXT("AddQuestStep", "Add Quest Step"));
	ParentGraph->Modify();

	UQuestGraphNode_Step* NewNode = NewObject<UQuestGraphNode_Step>(ParentGraph);
	NewNode->StepSubID = UQuestGraphSchema::AllocateSubID(ParentGraph);
	NewNode->NodePosX = FMath::RoundToInt(Location.X);
	NewNode->NodePosY = FMath::RoundToInt(Location.Y);
	NewNode->SetFlags(RF_Transactional);

	ParentGraph->AddNode(NewNode, /*bFromUI=*/true, bSelectNewNode);
	NewNode->CreateNewGuid();
	NewNode->PostPlacedNewNode();
	NewNode->AllocateDefaultPins();

	// Auto-wire from the source pin if dragged from an output
	if (FromPin && FromPin->Direction == EGPD_Output)
	{
		if (UEdGraphPin* InputPin = NewNode->GetInputPin())
			const_cast<UQuestGraphSchema*>(GetDefault<UQuestGraphSchema>())->TryCreateConnection(FromPin, InputPin);
	}

	return NewNode;
}

// ─────────────────────────────────────────────────────────────────────────────
// UQuestGraphSchema
// ─────────────────────────────────────────────────────────────────────────────

const FPinConnectionResponse UQuestGraphSchema::CanCreateConnection(
	const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (!A || !B)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("NullPin", "Invalid pin"));

	// Must be different directions
	if (A->Direction == B->Direction)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("SameDir", "Cannot connect pins of the same direction"));

	// Entry node only has an output — prevent connecting its output to another output
	const UEdGraphNode* OwnerA = A->GetOwningNode();
	const UEdGraphNode* OwnerB = B->GetOwningNode();
	if (OwnerA == OwnerB)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("SameNode", "Cannot connect a node to itself"));

	// Don't allow Input → Entry
	if (Cast<UQuestGraphNode_Entry>(B->GetOwningNode()) && B->Direction == EGPD_Input)
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, LOCTEXT("EntryNoInput", "The Entry node has no input"));

	// Linear / Optional output already linked → replace
	if (A->Direction == EGPD_Output && A->LinkedTo.Num() > 0)
	{
		const UQuestGraphNode_Step* StepNode = Cast<UQuestGraphNode_Step>(OwnerA);
		if (StepNode && (StepNode->StepType == EQuestStepType::Linear || StepNode->StepType == EQuestStepType::Optional))
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_BREAK_OTHERS_A,
				LOCTEXT("ReplaceLinear", "Replace existing connection"));
		}
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, FText::GetEmpty());
}

void UQuestGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	TSharedPtr<FQuestSchemaAction_NewStep> NewStepAction = MakeShareable(new FQuestSchemaAction_NewStep(
		LOCTEXT("QuestStepCategory", "Quest"),
		LOCTEXT("AddStep", "Add Quest Step"),
		LOCTEXT("AddStepTip", "Create a new quest step node."),
		0));
	ContextMenuBuilder.AddAction(NewStepAction);
}

void UQuestGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetContextMenuActions(Menu, Context);

	if (Context && Context->Node && !Context->bIsDebugging)
	{
		FToolMenuSection& Section = Menu->AddSection(TEXT("QuestNodeActions"), LOCTEXT("NodeActions", "Node Actions"));
		Section.AddMenuEntry(FGenericCommands::Get().Delete);
		Section.AddMenuEntry(FGenericCommands::Get().Cut);
		Section.AddMenuEntry(FGenericCommands::Get().Copy);
		Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
	}
}

void UQuestGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	// Create singleton Entry node only if one doesn't exist yet
	for (UEdGraphNode* Node : Graph.Nodes)
	{
		if (Cast<UQuestGraphNode_Entry>(Node))
			return;
	}

	FGraphNodeCreator<UQuestGraphNode_Entry> Creator(Graph);
	UQuestGraphNode_Entry* EntryNode = Creator.CreateNode();
	EntryNode->NodePosX = 0;
	EntryNode->NodePosY = 0;
	Creator.Finalize();
}

FLinearColor UQuestGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FLinearColor(0.9f, 0.9f, 0.1f); // yellow
}

void UQuestGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	const FScopedTransaction Transaction(LOCTEXT("BreakNodeLinks", "Break Node Links"));
	Super::BreakNodeLinks(TargetNode);
}

void UQuestGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotification) const
{
	const FScopedTransaction Transaction(LOCTEXT("BreakPinLinks", "Break Pin Links"));
	Super::BreakPinLinks(TargetPin, bSendsNodeNotification);
}

int32 UQuestGraphSchema::AllocateSubID(const UEdGraph* Graph)
{
	int32 MaxID = -1;
	for (const UEdGraphNode* Node : Graph->Nodes)
	{
		if (const UQuestGraphNode_Step* Step = Cast<UQuestGraphNode_Step>(Node))
			MaxID = FMath::Max(MaxID, Step->StepSubID);
	}
	return MaxID + 1;
}

#undef LOCTEXT_NAMESPACE

