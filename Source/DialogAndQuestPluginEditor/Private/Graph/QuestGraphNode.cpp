// Copyright 2024 Maximilien (Synock) Guislain

#include "Graph/QuestGraphNode.h"

#include "EdGraph/EdGraphPin.h"
#include "ToolMenu.h"
#include "ToolMenuSection.h"
#include "Graph/QuestGraphSchema.h"
#include "Graph/QuestGraph.h"
#include "Quest/QuestAsset.h"

#define LOCTEXT_NAMESPACE "QuestGraphNode"

const FName UQuestGraphNode_Step::PinCategory = TEXT("QuestFlow");
const FName UQuestGraphNode_Step::PinName_In   = TEXT("In");
const FName UQuestGraphNode_Step::PinName_Next  = TEXT("Next");

// ─────────────────────────────────────────────────────────────────────────────
// UQuestGraphNode_Entry
// ─────────────────────────────────────────────────────────────────────────────

void UQuestGraphNode_Entry::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UQuestGraphNode_Step::PinCategory, TEXT("Start"));
}

FText UQuestGraphNode_Entry::GetNodeTitle(ENodeTitleType::Type /*TitleType*/) const
{
	return LOCTEXT("EntryTitle", "START");
}

FLinearColor UQuestGraphNode_Entry::GetNodeTitleColor() const
{
	return FLinearColor(0.1f, 0.6f, 0.1f);
}

// ─────────────────────────────────────────────────────────────────────────────
// UQuestGraphNode_Step
// ─────────────────────────────────────────────────────────────────────────────

void UQuestGraphNode_Step::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, PinCategory, PinName_In);
	// Called from ReconstructNode during bulk graph build — caller notifies once
	RebuildOutputPins(/*bNotifyGraph=*/false);
}

FText UQuestGraphNode_Step::GetNodeTitle(ENodeTitleType::Type /*TitleType*/) const
{
	if (!StepData.StepTitle.IsEmpty())
		return StepData.StepTitle;
	return FText::FromString(FString::Printf(TEXT("Step %d"), StepSubID));
}

FLinearColor UQuestGraphNode_Step::GetNodeTitleColor() const
{
	if (StepData.FinishingStep)
		return FLinearColor(0.7f, 0.5f, 0.1f); // gold — finishing step

	switch (StepType)
	{
	case EQuestStepType::Branch:   return FLinearColor(0.6f, 0.2f, 0.6f); // purple
	case EQuestStepType::Parallel: return FLinearColor(0.1f, 0.4f, 0.8f); // blue
	case EQuestStepType::Optional: return FLinearColor(0.3f, 0.5f, 0.5f); // teal
	default:                        return FLinearColor(0.2f, 0.2f, 0.2f); // dark grey — linear
	}
}

FText UQuestGraphNode_Step::GetTooltipText() const
{
	static const UEnum* StepTypeEnum = StaticEnum<EQuestStepType>();
	FString TypeName = StepTypeEnum
		? StepTypeEnum->GetDisplayNameTextByValue((int64)StepType).ToString()
		: TEXT("?");
	return FText::FromString(FString::Printf(
		TEXT("SubID: %d\nType: %s\nFinishing: %s"),
		StepSubID, *TypeName, StepData.FinishingStep ? TEXT("Yes") : TEXT("No")));
}

void UQuestGraphNode_Step::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	FToolMenuSection& Section = Menu->AddSection(TEXT("QuestStepType"), LOCTEXT("StepTypeSection", "Step Type"));

	auto AddTypeAction = [&](EQuestStepType Type, const FText& Label)
	{
		Section.AddMenuEntry(
			FName(*Label.ToString()),
			Label, FText::GetEmpty(), FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([WeakNode = TWeakObjectPtr<UQuestGraphNode_Step>(const_cast<UQuestGraphNode_Step*>(this)), Type]()
				{
					if (UQuestGraphNode_Step* Node = WeakNode.Get())
					{
						Node->StepType           = Type;
						Node->StepData.StepType  = Type; // keep StepData in sync
						if (Type == EQuestStepType::Linear || Type == EQuestStepType::Optional)
							Node->BranchCount = 1;
						else if (Node->BranchCount < 2)
							Node->BranchCount = 2;

						Node->RebuildOutputPins();
					}
				}),
				FCanExecuteAction::CreateLambda([] { return true; }),
				FIsActionChecked::CreateLambda([WeakNode = TWeakObjectPtr<UQuestGraphNode_Step>(const_cast<UQuestGraphNode_Step*>(this)), Type]()
				{
					return WeakNode.IsValid() && WeakNode->StepType == Type;
				})
			),
			EUserInterfaceActionType::RadioButton
		);
	};

	AddTypeAction(EQuestStepType::Linear,   LOCTEXT("TypeLinear",   "Linear"));
	AddTypeAction(EQuestStepType::Branch,   LOCTEXT("TypeBranch",   "Branch"));
	AddTypeAction(EQuestStepType::Parallel, LOCTEXT("TypeParallel", "Parallel"));
	AddTypeAction(EQuestStepType::Optional, LOCTEXT("TypeOptional", "Optional"));

	if (StepType == EQuestStepType::Branch || StepType == EQuestStepType::Parallel)
	{
		FToolMenuSection& BranchSection = Menu->AddSection(TEXT("QuestBranchCount"), LOCTEXT("BranchCountSection", "Branches"));
		BranchSection.AddMenuEntry(
			TEXT("AddBranch"),
			LOCTEXT("AddBranch", "Add Output Pin"),
			LOCTEXT("AddBranchTip", "Add another output branch to this node."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([WeakNode = TWeakObjectPtr<UQuestGraphNode_Step>(const_cast<UQuestGraphNode_Step*>(this))]()
			{
				if (UQuestGraphNode_Step* Node = WeakNode.Get())
				{
					Node->BranchCount = FMath::Min(Node->BranchCount + 1, 8);
					Node->RebuildOutputPins();
				}
			}))
		);
		if (BranchCount > 2)
		{
			BranchSection.AddMenuEntry(
				TEXT("RemoveBranch"),
				LOCTEXT("RemoveBranch", "Remove Output Pin"),
				LOCTEXT("RemoveBranchTip", "Remove the last output branch from this node."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([WeakNode = TWeakObjectPtr<UQuestGraphNode_Step>(const_cast<UQuestGraphNode_Step*>(this))]()
				{
					if (UQuestGraphNode_Step* Node = WeakNode.Get())
					{
						Node->BranchCount = FMath::Max(Node->BranchCount - 1, 2);
						Node->RebuildOutputPins();
					}
				}))
			);
		}
	}
}

void UQuestGraphNode_Step::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();

	// Keep StepData.NextStepIDs live — rebuild from the current output pin wiring.
	StepData.NextStepIDs.Reset();
	for (UEdGraphPin* OutPin : GetOutputPins())
	{
		for (UEdGraphPin* Linked : OutPin->LinkedTo)
		{
			if (const UQuestGraphNode_Step* NextNode = Cast<UQuestGraphNode_Step>(Linked->GetOwningNode()))
				StepData.NextStepIDs.AddUnique(NextNode->StepSubID);
		}
	}

	// Propagate the updated NextStepIDs to the asset immediately
	SyncStepToAsset();
}

#if WITH_EDITOR
void UQuestGraphNode_Step::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.MemberProperty)
		return;

	const FName MemberName = PropertyChangedEvent.MemberProperty->GetFName();
	const FName PropName   = PropertyChangedEvent.GetPropertyName();

	bool bRebuildPins = false;

	if (MemberName == GET_MEMBER_NAME_CHECKED(UQuestGraphNode_Step, StepData))
	{
		// StepType changed in Details panel → sync StepType node field + rebuild pins
		if (PropName == GET_MEMBER_NAME_CHECKED(FQuestStep, StepType))
		{
			StepType = StepData.StepType;
			if (StepType == EQuestStepType::Linear || StepType == EQuestStepType::Optional)
				BranchCount = 1;
			else if (BranchCount < 2)
				BranchCount = 2;
			bRebuildPins = true;
		}

		// Any StepData change: sync to asset and repaint the node label
		SyncStepToAsset();

		if (!bRebuildPins)
		{
			// Repaint the canvas so StepTitle etc. update immediately
			if (UEdGraph* Graph = GetGraph())
				Graph->NotifyGraphChanged();
		}
	}

	// BranchCount edited directly in Details panel
	if (PropName == GET_MEMBER_NAME_CHECKED(UQuestGraphNode_Step, BranchCount))
		bRebuildPins = true;

	if (bRebuildPins)
		RebuildOutputPins(); // calls NotifyGraphChanged internally
}
#endif

UEdGraphPin* UQuestGraphNode_Step::GetInputPin() const
{
	for (UEdGraphPin* Pin : Pins)
		if (Pin && Pin->Direction == EGPD_Input)
			return Pin;
	return nullptr;
}

TArray<UEdGraphPin*> UQuestGraphNode_Step::GetOutputPins() const
{
	TArray<UEdGraphPin*> Out;
	for (UEdGraphPin* Pin : Pins)
		if (Pin && Pin->Direction == EGPD_Output)
			Out.Add(Pin);
	return Out;
}

void UQuestGraphNode_Step::RebuildOutputPins(bool bNotifyGraph)
{
	TMap<FName, TArray<UEdGraphPin*>> OldConnections;
	TArray<UEdGraphPin*> OldOutputPins;
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin && Pin->Direction == EGPD_Output)
		{
			OldConnections.FindOrAdd(Pin->PinName) = Pin->LinkedTo;
			OldOutputPins.Add(Pin);
		}
	}
	for (UEdGraphPin* Pin : OldOutputPins)
		RemovePin(Pin);

	TArray<FName> NewPinNames;
	if (StepType == EQuestStepType::Linear || StepType == EQuestStepType::Optional)
	{
		NewPinNames.Add(PinName_Next);
	}
	else
	{
		for (int32 i = 0; i < BranchCount; ++i)
			NewPinNames.Add(FName(*FString::Printf(TEXT("%s %c"),
				StepType == EQuestStepType::Branch ? TEXT("Branch") : TEXT("Track"),
				TEXT('A') + i)));
	}

	for (const FName& PinName : NewPinNames)
	{
		UEdGraphPin* NewPin = CreatePin(EGPD_Output, PinCategory, PinName);
		if (TArray<UEdGraphPin*>* Old = OldConnections.Find(PinName))
			for (UEdGraphPin* Target : *Old)
				if (Target && !Target->bOrphanedPin)
					NewPin->MakeLinkTo(Target);
	}

	// Notify the graph so SGraphEditor redraws this node with the new pin layout
	if (bNotifyGraph)
	{
		if (UEdGraph* Graph = GetGraph())
			Graph->NotifyGraphChanged();
	}
}

void UQuestGraphNode_Step::SyncStepToAsset()
{
	UQuestGraph* QGraph = Cast<UQuestGraph>(GetGraph());
	if (!QGraph)
		return;

	UQuestAsset* Asset = QGraph->GetOwnerAsset();
	if (!Asset)
		return;

	for (FQuestStep& Step : Asset->Steps)
	{
		if (Step.QuestSubID != StepSubID)
			continue;

		// Preserve topology fields that are owned by the graph, not the Details panel
		const int64          SavedQuestID    = Asset->QuestID;
		const int32          SavedSubID      = Step.QuestSubID;
		const EQuestStepType SavedStepType   = Step.StepType;
		const TArray<int32>  SavedNextIDs    = Step.NextStepIDs;

		Step = StepData; // copy all content fields

		Step.QuestID     = SavedQuestID;
		Step.QuestSubID  = SavedSubID;
		Step.StepType    = SavedStepType;
		Step.NextStepIDs = StepData.NextStepIDs; // keep live-wired IDs

		Asset->MarkPackageDirty();
		break;
	}
}

#undef LOCTEXT_NAMESPACE

