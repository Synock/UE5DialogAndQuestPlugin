// Copyright 2022 Maximilien (Synock) Guislain

#include "DetailCustomization/DialogAssetDetailCustomization.h"

#include "Dialog/DialogAsset.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "DialogAssetDetailCustomization"

TSharedRef<IDetailCustomization> FDialogAssetDetailCustomization::MakeInstance()
{
	return MakeShareable(new FDialogAssetDetailCustomization());
}

void FDialogAssetDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CachedDetailBuilder = &DetailBuilder;

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 1)
		DialogAssetPtr = Cast<UDialogAsset>(Objects[0].Get());

	IDetailCategoryBuilder& ToolsCategory = DetailBuilder.EditCategory(
		TEXT("Dialog Tools"),
		LOCTEXT("DialogToolsCategory", "Dialog Tools"),
		ECategoryPriority::Important);

	// --- Button row ---
	ToolsCategory.AddCustomRow(LOCTEXT("ToolsRow", "Tools"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 2.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("AssignTopicIDs", "Assign Next Topic IDs"))
			.ToolTipText(LOCTEXT("AssignTopicIDsTip",
				"Sets Id = 0 topics to the next available ID (current max + 1, +2...).\n"
				"Topic IDs must be globally unique across all DialogAssets."))
			.OnClicked(this, &FDialogAssetDetailCustomization::OnAssignNextTopicIDs)
		]
	];

	// --- Inline validation ---
	if (!DialogAssetPtr.IsValid())
		return;

	UDialogAsset* Asset = DialogAssetPtr.Get();
	TArray<FString> Warnings;

	// Collect all IDs from shared assets (warn on cross-asset collisions)
	TMap<int64, FString> SharedIDs;  // ID → shared asset name
	for (const TSoftObjectPtr<UDialogAsset>& SharedRef : Asset->SharedTopicAssets)
	{
		if (UDialogAsset* Shared = SharedRef.Get())
		{
			for (const FDialogTopicStruct& T : Shared->Topics)
			{
				if (!SharedIDs.Contains(T.Id))
					SharedIDs.Add(T.Id, Shared->AssetName);
			}
		}
	}

	TMap<int64, int32> IDCount;
	for (const FDialogTopicStruct& Topic : Asset->Topics)
		IDCount.FindOrAdd(Topic.Id)++;

	// Unset IDs
	const int32* ZeroCount = IDCount.Find(0);
	if (ZeroCount && *ZeroCount > 0)
		Warnings.Add(FString::Printf(
			TEXT("%d topic(s) have Id = 0. Use \"Assign Next Topic IDs\" to fix."), *ZeroCount));

	// Duplicate non-zero IDs within this asset
	for (const auto& Pair : IDCount)
	{
		if (Pair.Key != 0 && Pair.Value > 1)
		{
			FString Names;
			for (const FDialogTopicStruct& Topic : Asset->Topics)
				if (Topic.Id == Pair.Key)
					Names += TEXT(" \"") + Topic.Topic + TEXT("\"");
			Warnings.Add(FString::Printf(
				TEXT("Duplicate topic ID %lld (%d occurrences):%s"), Pair.Key, Pair.Value, *Names));
		}
	}

	// Cross-asset ID collisions with shared assets
	for (const FDialogTopicStruct& Topic : Asset->Topics)
	{
		if (Topic.Id != 0)
		{
			if (const FString* SharedName = SharedIDs.Find(Topic.Id))
			{
				Warnings.Add(FString::Printf(
					TEXT("Topic \"%s\" (Id %lld) collides with shared asset \"%s\". Use \"Assign Next Topic IDs\"."),
					*Topic.Topic, Topic.Id, **SharedName));
			}
		}
	}

	// Deprecated numeric Quest ID migration warnings.
	// Only flag topics that have quest-driven fields populated but are missing the asset pointer —
	// topics with no quest involvement at all are not flagged.
	int32 DeprecatedQuestIDCount = 0;
	for (const FDialogTopicStruct& Topic : Asset->Topics)
	{
		// Condition side: topic has a quest state or step filter but no Quest asset.
		const bool bHasConditionQuestData =
			Topic.TopicCondition.RequiredQuestState != EQuestState::Unknown ||
			Topic.TopicCondition.bUseStepFilter ||
			Topic.TopicCondition.MinimumStepID != 0;
		const bool bConditionDeprecated = bHasConditionQuestData && !Topic.TopicCondition.Quest;

		// Consequence side: topic changes quest state or advances a step but no Quest asset.
		const bool bHasConsequenceQuestData =
			Topic.Consequence.NewQuestState != EQuestState::Unknown ||
			Topic.Consequence.bAdvanceStep;
		const bool bConsequenceDeprecated = bHasConsequenceQuestData && !Topic.Consequence.Quest;

		if (bConditionDeprecated || bConsequenceDeprecated)
			++DeprecatedQuestIDCount;
	}

	if (DeprecatedQuestIDCount > 0)
		Warnings.Add(FString::Printf(
			TEXT("%d topic(s) still use deprecated numeric Quest IDs. Open each Topic and assign the Quest asset pointer."),
			DeprecatedQuestIDCount));

	int32 UngatedAdvanceCount = 0;
	for (const FDialogTopicStruct& Topic : Asset->Topics)
	{
		const bool bHasExplicitStepFilter = Topic.TopicCondition.bUseStepFilter ||
			Topic.TopicCondition.MinimumStepID != 0;
		if (Topic.Consequence.bAdvanceStep && Topic.Consequence.Quest && !bHasExplicitStepFilter)
			++UngatedAdvanceCount;
	}

	if (UngatedAdvanceCount > 0)
		Warnings.Add(FString::Printf(
			TEXT("%d advancing topic(s) have no explicit step filter. They can advance whichever quest step is current."),
			UngatedAdvanceCount));

	if (Warnings.IsEmpty())
		return;

	FString AllWarnings;
	for (const FString& W : Warnings)
		AllWarnings += TEXT("⚠  ") + W + TEXT("\n");

	ToolsCategory.AddCustomRow(LOCTEXT("ValidationWarnings", "Validation Warnings"))
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.4f, 0.2f, 0.f, 0.5f))
		.Padding(6.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(AllWarnings.TrimEnd()))
			.ColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.75f, 0.2f)))
			.AutoWrapText(true)
		]
	];
}

FReply FDialogAssetDetailCustomization::OnAssignNextTopicIDs()
{
	if (!DialogAssetPtr.IsValid())
		return FReply::Handled();

	UDialogAsset* Asset = DialogAssetPtr.Get();

	// Include IDs from shared assets so we never accidentally reuse a shared topic ID.
	int64 MaxID = 0;
	for (const TSoftObjectPtr<UDialogAsset>& SharedRef : Asset->SharedTopicAssets)
	{
		if (UDialogAsset* Shared = SharedRef.Get())  // only already-loaded assets — no sync load in editor
		{
			for (const FDialogTopicStruct& T : Shared->Topics)
				MaxID = FMath::Max(MaxID, T.Id);
		}
	}
	for (const FDialogTopicStruct& Topic : Asset->Topics)
		MaxID = FMath::Max(MaxID, Topic.Id);

	int64 NextID = MaxID + 1;
	for (FDialogTopicStruct& Topic : Asset->Topics)
	{
		if (Topic.Id == 0)
			Topic.Id = NextID++;
	}

	(void)Asset->MarkPackageDirty();

	if (CachedDetailBuilder)
		CachedDetailBuilder->ForceRefreshDetails();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE




