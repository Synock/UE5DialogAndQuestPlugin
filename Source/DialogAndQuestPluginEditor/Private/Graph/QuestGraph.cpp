// Copyright 2024 Maximilien (Synock) Guislain

#include "Graph/QuestGraph.h"

#include "Graph/QuestGraphNode.h"
#include "Graph/QuestGraphSchema.h"
#include "Quest/QuestAsset.h"
#include "Quest/QuestData.h"

// ─────────────────────────────────────────────────────────────────────────────
// BuildFromAsset
// ─────────────────────────────────────────────────────────────────────────────

void UQuestGraph::BuildFromAsset(UQuestAsset* Asset)
{
	if (!Asset)
		return;

	Schema     = UQuestGraphSchema::StaticClass();
	OwnerAsset = Asset; // back-reference for live node→asset sync

	// ── 1. Ensure Entry node exists ────────────────────────────────────────
	if (!GetEntryNode())
	{
		FGraphNodeCreator<UQuestGraphNode_Entry> EntryCreator(*this);
		UQuestGraphNode_Entry* EntryNode = EntryCreator.CreateNode();
		EntryNode->NodePosX = 0;
		EntryNode->NodePosY = 0;
		EntryCreator.Finalize();
	}

	// ── 2. Collect existing step nodes by SubID ────────────────────────────
	TMap<int32, UQuestGraphNode_Step*> ExistingNodes;
	for (UEdGraphNode* Node : Nodes)
	{
		if (UQuestGraphNode_Step* StepNode = Cast<UQuestGraphNode_Step>(Node))
			ExistingNodes.Add(StepNode->StepSubID, StepNode);
	}

	// ── 3. Create or update a node for each step ───────────────────────────
	TMap<int32, UQuestGraphNode_Step*> ActiveNodes;
	for (const FQuestStep& Step : Asset->Steps)
	{
		UQuestGraphNode_Step* Node = ExistingNodes.FindRef(Step.QuestSubID);
		if (!Node)
		{
			FGraphNodeCreator<UQuestGraphNode_Step> Creator(*this);
			Node = Creator.CreateNode(/*bSelectNewNode=*/false);
			Creator.Finalize();
		}

		// Populate full step data — this is the editable surface in the Details panel
		Node->StepData      = Step;
		Node->StepSubID     = Step.QuestSubID;
		Node->StepType      = Step.StepType;
		Node->BranchCount   = (Step.StepType == EQuestStepType::Linear ||
		                       Step.StepType == EQuestStepType::Optional)
		                      ? 1
		                      : FMath::Max(Step.NextStepIDs.Num(), 2);
		Node->ReconstructNode();

		ActiveNodes.Add(Step.QuestSubID, Node);
	}

	// ── 4. Remove nodes whose SubIDs no longer exist in the asset ─────────
	TArray<UEdGraphNode*> ToRemove;
	for (UEdGraphNode* Node : Nodes)
	{
		if (UQuestGraphNode_Step* StepNode = Cast<UQuestGraphNode_Step>(Node))
		{
			if (!ActiveNodes.Contains(StepNode->StepSubID))
				ToRemove.Add(StepNode);
		}
	}
	for (UEdGraphNode* Node : ToRemove)
		RemoveNode(Node);

	// ── 5. Rebuild connections from Steps[] ────────────────────────────────
	// Clear all existing wires first so we start clean
	for (UEdGraphNode* Node : Nodes)
	{
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (Pin)
				Pin->BreakAllPinLinks(/*bNotifyConnected=*/false);
		}
	}

	// Entry → first step (SubID 0, or first in array)
	UQuestGraphNode_Entry* EntryNode = GetEntryNode();
	if (EntryNode && EntryNode->Pins.Num() > 0 && Asset->Steps.Num() > 0)
	{
		int32 FirstSubID = Asset->Steps[0].QuestSubID;
		if (UQuestGraphNode_Step* FirstStep = ActiveNodes.FindRef(FirstSubID))
		{
			if (UEdGraphPin* InputPin = FirstStep->GetInputPin())
				EntryNode->Pins[0]->MakeLinkTo(InputPin);
		}
	}

	// Wire each step's output pins
	for (const FQuestStep& Step : Asset->Steps)
	{
		UQuestGraphNode_Step* FromNode = ActiveNodes.FindRef(Step.QuestSubID);
		if (!FromNode)
			continue;

		TArray<UEdGraphPin*> OutPins = FromNode->GetOutputPins();

		if (Step.StepType == EQuestStepType::Linear || Step.StepType == EQuestStepType::Optional)
		{
			// Linear: find this step's index, then wire to the next in array order
			if (OutPins.Num() > 0)
			{
				int32 FoundIdx = INDEX_NONE;
				for (int32 i = 0; i < Asset->Steps.Num(); ++i)
				{
					if (Asset->Steps[i].QuestSubID == Step.QuestSubID)
					{
						FoundIdx = i;
						break;
					}
				}
				if (FoundIdx != INDEX_NONE && FoundIdx + 1 < Asset->Steps.Num())
				{
					int32 NextSubID = Asset->Steps[FoundIdx + 1].QuestSubID;
					if (UQuestGraphNode_Step* NextNode = ActiveNodes.FindRef(NextSubID))
					{
						if (UEdGraphPin* NextInput = NextNode->GetInputPin())
							OutPins[0]->MakeLinkTo(NextInput);
					}
				}
			}
		}
		else // Branch / Parallel: use NextStepIDs
		{
			for (int32 i = 0; i < Step.NextStepIDs.Num() && i < OutPins.Num(); ++i)
			{
				if (UQuestGraphNode_Step* TargetNode = ActiveNodes.FindRef(Step.NextStepIDs[i]))
				{
					if (UEdGraphPin* NextInput = TargetNode->GetInputPin())
						OutPins[i]->MakeLinkTo(NextInput);
				}
			}
		}
	}

	// ── 6. Auto-arrange on first open (no saved layout yet) ───────────────
	if (!bHasSavedLayout)
		AutoArrangeNodes();

	// Single graph-change notification after the full rebuild
	NotifyGraphChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// CompileToAsset
// ─────────────────────────────────────────────────────────────────────────────

bool UQuestGraph::CompileToAsset(UQuestAsset* Asset, TArray<FString>& OutErrors)
{
	if (!Asset)
	{
		OutErrors.Add(TEXT("No asset to compile into."));
		return false;
	}

	// ── 1. Collect nodes & build SubID → FQuestStep map ───────────────────
	TMap<int32, UQuestGraphNode_Step*> NodeMap;
	for (UEdGraphNode* Node : Nodes)
	{
		if (UQuestGraphNode_Step* StepNode = Cast<UQuestGraphNode_Step>(Node))
			NodeMap.Add(StepNode->StepSubID, StepNode);
	}

	if (NodeMap.IsEmpty() && Asset->Steps.IsEmpty())
		return true; // empty quest, nothing to do

	// ── 2. Validate: entry node exists ────────────────────────────────────
	UQuestGraphNode_Entry* EntryNode = GetEntryNode();
	if (!EntryNode)
	{
		OutErrors.Add(TEXT("Graph has no Entry node. Cannot compile."));
		return false;
	}

	// ── 3. Topological order from Entry via BFS ────────────────────────────
	TArray<int32> OrderedSubIDs;
	TSet<int32>   Visited;
	TQueue<int32> Queue;

	// Seed from Entry's connection
	if (EntryNode->Pins.Num() > 0)
	{
		for (UEdGraphPin* Linked : EntryNode->Pins[0]->LinkedTo)
		{
			if (UQuestGraphNode_Step* Step = Cast<UQuestGraphNode_Step>(Linked->GetOwningNode()))
			{
				if (!Visited.Contains(Step->StepSubID))
				{
					Queue.Enqueue(Step->StepSubID);
					Visited.Add(Step->StepSubID);
				}
			}
		}
	}

	while (!Queue.IsEmpty())
	{
		int32 CurID;
		Queue.Dequeue(CurID);
		OrderedSubIDs.Add(CurID);

		UQuestGraphNode_Step* CurNode = NodeMap.FindRef(CurID);
		if (!CurNode)
			continue;

		for (UEdGraphPin* OutPin : CurNode->GetOutputPins())
		{
			for (UEdGraphPin* Linked : OutPin->LinkedTo)
			{
				if (UQuestGraphNode_Step* NextStep = Cast<UQuestGraphNode_Step>(Linked->GetOwningNode()))
				{
					if (!Visited.Contains(NextStep->StepSubID))
					{
						Visited.Add(NextStep->StepSubID);
						Queue.Enqueue(NextStep->StepSubID);
					}
				}
			}
		}
	}

	// Warn about orphan nodes (not reachable from Entry) — don't fail, just warn
	for (auto& Pair : NodeMap)
	{
		if (!Visited.Contains(Pair.Key))
		{
			OutErrors.Add(FString::Printf(
				TEXT("Warning: Step SubID %d is not reachable from the Entry node and will be moved to the end."),
				Pair.Key));
			OrderedSubIDs.Add(Pair.Key);
		}
	}

	// ── 4. Validate finishing step ─────────────────────────────────────────
	int32 FinishCount = 0;
	for (auto& Pair : NodeMap)
		if (Pair.Value->StepData.FinishingStep) ++FinishCount;
	if (NodeMap.Num() > 0 && FinishCount == 0)
		OutErrors.Add(TEXT("Warning: No step is marked as Finishing. FinishingStep should be true on the last step."));
	if (FinishCount > 1)
		OutErrors.Add(FString::Printf(TEXT("Warning: %d steps are marked as FinishingStep. Typically only the last should be."), FinishCount));

	// ── 6. Rebuild Asset->Steps in topological order ───────────────────────
	TArray<FQuestStep> NewSteps;
	NewSteps.Reserve(OrderedSubIDs.Num());

	for (int32 SubID : OrderedSubIDs)
	{
		UQuestGraphNode_Step* Node = NodeMap.FindRef(SubID);
		if (!Node)
			continue;

		// Start from Node->StepData (fully edited via Details panel)
		FQuestStep Step    = Node->StepData;
		// Override graph-managed topology fields
		Step.QuestID       = Asset->QuestID;
		Step.QuestSubID    = SubID;
		Step.StepType      = Node->StepType;
		// FinishingStep lives in StepData — already correct

		// Rebuild NextStepIDs from output pin connections
		Step.NextStepIDs.Reset();
		for (UEdGraphPin* OutPin : Node->GetOutputPins())
		{
			for (UEdGraphPin* Linked : OutPin->LinkedTo)
			{
				if (UQuestGraphNode_Step* NextNode = Cast<UQuestGraphNode_Step>(Linked->GetOwningNode()))
					Step.NextStepIDs.AddUnique(NextNode->StepSubID);
			}
		}

		NewSteps.Add(Step);
	}

	Asset->Modify();
	Asset->Steps = MoveTemp(NewSteps);

	bHasSavedLayout = true;
	return OutErrors.IsEmpty() || OutErrors.FilterByPredicate([](const FString& E) { return !E.StartsWith(TEXT("Warning:")); }).IsEmpty();
}

// ─────────────────────────────────────────────────────────────────────────────
// AutoArrangeNodes  — left-to-right BFS layout
// ─────────────────────────────────────────────────────────────────────────────

void UQuestGraph::AutoArrangeNodes()
{
	constexpr float HSpacing = 320.f;
	constexpr float VSpacing = 160.f;
	constexpr float NodeWidth = 260.f; // approximate

	// BFS from Entry
	UQuestGraphNode_Entry* EntryNode = GetEntryNode();
	if (!EntryNode)
		return;

	EntryNode->NodePosX = 0;
	EntryNode->NodePosY = 0;

	TMap<UEdGraphNode*, int32>   DepthMap;  // column
	TMap<int32, TArray<UEdGraphNode*>> Columns; // column → node list

	TQueue<UEdGraphNode*> Queue;
	TSet<UEdGraphNode*>   Visited;

	DepthMap.Add(EntryNode, 0);
	Columns.FindOrAdd(0).Add(EntryNode);
	Queue.Enqueue(EntryNode);
	Visited.Add(EntryNode);

	while (!Queue.IsEmpty())
	{
		UEdGraphNode* Current = nullptr;
		Queue.Dequeue(Current);
		int32 CurrentDepth = DepthMap[Current];

		// Iterate output pins
		for (UEdGraphPin* Pin : Current->Pins)
		{
			if (!Pin || Pin->Direction != EGPD_Output)
				continue;
			for (UEdGraphPin* Linked : Pin->LinkedTo)
			{
				if (!Linked)
					continue;
				UEdGraphNode* Next = Linked->GetOwningNode();
				if (!Visited.Contains(Next))
				{
					Visited.Add(Next);
					int32 NextDepth = CurrentDepth + 1;
					DepthMap.Add(Next, FMath::Max(NextDepth, DepthMap.FindRef(Next)));
					Columns.FindOrAdd(NextDepth).Add(Next);
					Queue.Enqueue(Next);
				}
			}
		}
	}

	// Position nodes
	for (auto& ColPair : Columns)
	{
		int32 Col = ColPair.Key;
		TArray<UEdGraphNode*>& ColNodes = ColPair.Value;
		float TotalHeight = ColNodes.Num() * VSpacing;
		float StartY = -TotalHeight * 0.5f;

		for (int32 i = 0; i < ColNodes.Num(); ++i)
		{
			ColNodes[i]->NodePosX = FMath::RoundToInt(Col * (NodeWidth + HSpacing));
			ColNodes[i]->NodePosY = FMath::RoundToInt(StartY + i * VSpacing);
		}
	}

	bHasSavedLayout = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

UQuestGraphNode_Entry* UQuestGraph::GetEntryNode() const
{
	for (UEdGraphNode* Node : Nodes)
	{
		if (UQuestGraphNode_Entry* Entry = Cast<UQuestGraphNode_Entry>(Node))
			return Entry;
	}
	return nullptr;
}

UQuestGraphNode_Step* UQuestGraph::FindStepNode(int32 SubID) const
{
	for (UEdGraphNode* Node : Nodes)
	{
		if (UQuestGraphNode_Step* Step = Cast<UQuestGraphNode_Step>(Node))
		{
			if (Step->StepSubID == SubID)
				return Step;
		}
	}
	return nullptr;
}

