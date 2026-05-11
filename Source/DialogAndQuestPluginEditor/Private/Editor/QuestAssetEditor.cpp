// Copyright 2024 Maximilien (Synock) Guislain

#include "Editor/QuestAssetEditor.h"

#include "Graph/QuestGraph.h"
#include "Graph/QuestGraphSchema.h"
#include "Graph/QuestGraphNode.h"
#include "Quest/QuestAsset.h"

#include "EdGraph/EdGraph.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Toolkits/IToolkitHost.h"
#include "ToolMenus.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "QuestAssetEditor"

const FName FQuestAssetEditor::TabId_Graph   = TEXT("QuestAssetEditor_Graph");
const FName FQuestAssetEditor::TabId_Details = TEXT("QuestAssetEditor_Details");

// ─────────────────────────────────────────────────────────────────────────────
// Static entry point
// ─────────────────────────────────────────────────────────────────────────────

void FQuestAssetEditor::OpenEditor(EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& Host, UQuestAsset* Asset)
{
	TSharedRef<FQuestAssetEditor> Editor = MakeShareable(new FQuestAssetEditor());
	Editor->InitEditor(Mode, Host, Asset);
}

// ─────────────────────────────────────────────────────────────────────────────
// InitEditor
// ─────────────────────────────────────────────────────────────────────────────

void FQuestAssetEditor::InitEditor(EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& Host, UQuestAsset* Asset)
{
	QuestAsset = Asset;

	// ── Build / ensure the editor graph ───────────────────────────────────
	if (!Asset->EditorGraph)
	{
		Asset->EditorGraph = NewObject<UQuestGraph>(Asset, UQuestGraph::StaticClass(),
			TEXT("QuestEditorGraph"), RF_Transactional);
		Asset->EditorGraph->Schema = UQuestGraphSchema::StaticClass();
		GetDefault<UQuestGraphSchema>()->CreateDefaultNodesForGraph(*Asset->EditorGraph);
	}

	UQuestGraph* Graph = CastChecked<UQuestGraph>(Asset->EditorGraph);
	Graph->BuildFromAsset(Asset);

	// ── Details view ──────────────────────────────────────────────────────
	FPropertyEditorModule& PropModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.bHideSelectionTip      = true;
	DetailsArgs.bAllowSearch           = true;
	DetailsArgs.NotifyHook             = nullptr;
	DetailsArgs.NameAreaSettings       = FDetailsViewArgs::HideNameArea;
	DetailsView = PropModule.CreateDetailView(DetailsArgs);
	DetailsView->SetObject(Asset);

	// ── Graph editor widget ────────────────────────────────────────────────
	SGraphEditor::FGraphEditorEvents GraphEvents = MakeGraphEvents();

	FGraphAppearanceInfo Appearance;
	Appearance.CornerText = LOCTEXT("QuestFlowLabel", "QUEST FLOW");

	GraphEditorWidget = SNew(SGraphEditor)
		.GraphToEdit(Graph)
		.GraphEvents(GraphEvents)
		.Appearance(Appearance)
		.IsEditable(true);

	// ── Layout ────────────────────────────────────────────────────────────
	const TSharedRef<FTabManager::FLayout> Layout =
		FTabManager::NewLayout("QuestAssetEditor_Layout_v2")
		->AddArea(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.72f)
				->AddTab(TabId_Graph, ETabState::OpenedTab)
			)
			->Split(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.28f)
				->AddTab(TabId_Details, ETabState::OpenedTab)
			)
		);

	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar        = true;
	FAssetEditorToolkit::InitAssetEditor(Mode, Host, FName("QuestAssetEditor"),
		Layout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, Asset);

	ExtendToolbar();
	RegenerateMenusAndToolbars();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tab spawners
// ─────────────────────────────────────────────────────────────────────────────

void FQuestAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("WorkspaceMenu_Quest", "Quest Editor"));

	InTabManager->RegisterTabSpawner(TabId_Graph, FOnSpawnTab::CreateSP(this, &FQuestAssetEditor::SpawnTab_Graph))
		.SetDisplayName(LOCTEXT("GraphTab", "Graph"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(TabId_Details, FOnSpawnTab::CreateSP(this, &FQuestAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FQuestAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(TabId_Graph);
	InTabManager->UnregisterTabSpawner(TabId_Details);
}

TSharedRef<SDockTab> FQuestAssetEditor::SpawnTab_Graph(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("GraphTabLabel", "Quest Flow"))
		[
			GraphEditorWidget.IsValid()
			? GraphEditorWidget.ToSharedRef()
			: SNullWidget::NullWidget
		];
}

TSharedRef<SDockTab> FQuestAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTabLabel", "Details"))
		[
			DetailsView.IsValid()
			? DetailsView.ToSharedRef()
			: SNullWidget::NullWidget
		];
}

// ─────────────────────────────────────────────────────────────────────────────
// Toolbar
// ─────────────────────────────────────────────────────────────────────────────

void FQuestAssetEditor::ExtendToolbar()
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(GetToolMenuToolbarName());
	FToolMenuSection& Section = ToolbarMenu->AddSection(TEXT("QuestCommands"));

	Section.AddEntry(FToolMenuEntry::InitToolBarButton(
		TEXT("CompileQuest"),
		FUIAction(
			FExecuteAction::CreateLambda([WeakEditor = TWeakPtr<FQuestAssetEditor>(SharedThis(this))]()
			{
				if (TSharedPtr<FQuestAssetEditor> Editor = WeakEditor.Pin())
					Editor->OnCompileClicked();
			}),
			FCanExecuteAction::CreateSP(this, &FQuestAssetEditor::CanCompile)
		),
		LOCTEXT("CompileLabel", "Compile"),
		LOCTEXT("CompileTip",
			"Compile the graph into the Steps array.\n"
			"Updates step order, StepType, NextStepIDs and FinishingStep.\n"
			"Step data (descriptions, items, rewards) is preserved."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "AssetEditor.SaveAsset")
	));
}

void FQuestAssetEditor::OnCompileClicked()
{
	if (!QuestAsset.Get())
		return;

	UQuestGraph* Graph = Cast<UQuestGraph>(QuestAsset->EditorGraph);
	if (!Graph)
		return;

	TArray<FString> Errors;
	const FScopedTransaction Transaction(LOCTEXT("CompileQuest", "Compile Quest Graph"));
	const bool bSuccess = Graph->CompileToAsset(QuestAsset.Get(), Errors);

	if (!Errors.IsEmpty())
	{
		FString Msg = FString::Join(Errors, TEXT("\n"));
		UE_LOG(LogTemp, Warning, TEXT("Quest compile: %s"), *Msg);
	}

	if (bSuccess)
		QuestAsset->MarkPackageDirty();

	if (DetailsView.IsValid())
		DetailsView->SetObject(QuestAsset.Get(), /*bForceRefresh=*/true);
}

bool FQuestAssetEditor::CanCompile() const
{
	return QuestAsset.Get() != nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Graph events
// ─────────────────────────────────────────────────────────────────────────────

SGraphEditor::FGraphEditorEvents FQuestAssetEditor::MakeGraphEvents()
{
	SGraphEditor::FGraphEditorEvents Events;
	Events.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(
		this, &FQuestAssetEditor::OnSelectedNodesChanged);
	return Events;
}

void FQuestAssetEditor::OnSelectedNodesChanged(const TSet<UObject*>& SelectedObjects)
{
	if (!DetailsView.IsValid())
		return;

	// If exactly one step node is selected, show its backing FQuestStep in Details.
	// Otherwise fall back to showing the whole asset.
	TArray<UObject*> SelectedNodes;
	for (UObject* Obj : SelectedObjects)
	{
		if (Cast<UQuestGraphNode_Step>(Obj))
			SelectedNodes.Add(Obj);
	}

	if (SelectedNodes.Num() == 1)
		DetailsView->SetObjects(SelectedNodes);
	else
		DetailsView->SetObject(QuestAsset.Get());
}

// ─────────────────────────────────────────────────────────────────────────────
// Toolkit identification
// ─────────────────────────────────────────────────────────────────────────────

FText FQuestAssetEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Quest Editor");
}

FName FQuestAssetEditor::GetToolkitFName() const
{
	return FName("QuestAssetEditor");
}

FString FQuestAssetEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("Quest ");
}

FLinearColor FQuestAssetEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.6f, 0.5f, 0.05f);
}

bool FQuestAssetEditor::OnRequestClose(EAssetEditorCloseReason CloseReason)
{
	return FAssetEditorToolkit::OnRequestClose(CloseReason);
}

#undef LOCTEXT_NAMESPACE



