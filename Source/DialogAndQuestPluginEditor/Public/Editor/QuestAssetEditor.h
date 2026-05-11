// Copyright 2024 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "GraphEditor.h"

class UQuestAsset;
class UQuestGraph;
class IDetailsView;

/**
 * Custom asset editor for UQuestAsset.
 *
 * Two-tab layout:
 *   Graph   — SGraphEditor showing the quest step flow (Branch / Parallel wiring)
 *   Details — standard property panel for per-step data (descriptions, items, rewards)
 *
 * A toolbar "Compile" button calls UQuestGraph::CompileToAsset() and marks the package dirty.
 * BuildFromAsset() is called on open so existing Step arrays round-trip correctly.
 */
class FQuestAssetEditor : public FAssetEditorToolkit
{
public:
	static void OpenEditor(EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& Host, UQuestAsset* Asset);
	void InitEditor(EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& Host, UQuestAsset* Asset);

	// ── FAssetEditorToolkit ────────────────────────────────────────────────
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual FText GetBaseToolkitName() const override;
	virtual FName GetToolkitFName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual bool OnRequestClose(EAssetEditorCloseReason CloseReason) override;

	// ── Tab IDs ───────────────────────────────────────────────────────────
	static const FName TabId_Graph;
	static const FName TabId_Details;

private:
	TSharedRef<SDockTab> SpawnTab_Graph(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	/** Wires SGraphEditor event handlers. */
	SGraphEditor::FGraphEditorEvents MakeGraphEvents();

	void ExtendToolbar();

	void OnCompileClicked();
	bool CanCompile() const;

	/** Called when the user selects a node; updates the Details panel. */
	void OnSelectedNodesChanged(const TSet<UObject*>& SelectedObjects);

	TObjectPtr<UQuestAsset>      QuestAsset;
	TSharedPtr<SGraphEditor>     GraphEditorWidget;
	TSharedPtr<IDetailsView>     DetailsView;
};


