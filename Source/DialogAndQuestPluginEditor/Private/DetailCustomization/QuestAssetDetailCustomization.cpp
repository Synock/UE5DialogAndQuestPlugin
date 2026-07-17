// Copyright 2022 Maximilien (Synock) Guislain

#include "DetailCustomization/QuestAssetDetailCustomization.h"

#include "Quest/QuestAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "QuestAssetDetailCustomization"

TSharedRef<IDetailCustomization> FQuestAssetDetailCustomization::MakeInstance()
{
	return MakeShareable(new FQuestAssetDetailCustomization());
}

void FQuestAssetDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CachedDetailBuilder = &DetailBuilder;

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 1)
		QuestAssetPtr = Cast<UQuestAsset>(Objects[0].Get());

	IDetailCategoryBuilder& ToolsCategory = DetailBuilder.EditCategory(
		TEXT("Quest Tools"),
		LOCTEXT("QuestToolsCategory", "Quest Tools"),
		ECategoryPriority::Important);

	// --- Buttons row ---
	ToolsCategory.AddCustomRow(LOCTEXT("ToolsRow", "Tools"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 2.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("AssignNextQuestID", "Assign Next Quest ID"))
			.ToolTipText(LOCTEXT("AssignNextQuestIDTip",
				"Assigns the selected quest the next ID after the highest non-zero QuestID\n"
				"found in project Quest Assets, then syncs every step. This does not reserve\n"
				"an ID in the persistence database."))
			.OnClicked(this, &FQuestAssetDetailCustomization::OnAssignNextQuestID)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 2.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SyncQuestIDs", "Sync Step QuestIDs"))
			.ToolTipText(LOCTEXT("SyncQuestIDsTip",
				"Copies this asset's QuestID into every FQuestStep::QuestID field.\n"
				"Run this any time you add steps or change the top-level QuestID."))
			.OnClicked(this, &FQuestAssetDetailCustomization::OnSyncStepQuestIDs)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 2.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SequenceSubIDs", "Sequence SubIDs"))
			.ToolTipText(LOCTEXT("SequenceSubIDsTip",
				"Renumbers QuestSubID 0, 1, 2 … in array order.\n"
				"Use after reordering or deleting steps."))
			.OnClicked(this, &FQuestAssetDetailCustomization::OnAutoSequenceSubIDs)
		]
	];

	// --- Inline validation ---
	if (!QuestAssetPtr.IsValid())
		return;

	UQuestAsset* Asset = QuestAssetPtr.Get();
	TArray<FString> Warnings;

	// Check that every step's QuestID matches the parent
	for (const FQuestStep& Step : Asset->Steps)
	{
		if (Step.QuestID != Asset->QuestID && Asset->QuestID != 0)
		{
			Warnings.Add(FString::Printf(
				TEXT("Step SubID %d: QuestID is %lld but asset QuestID is %lld. Use \"Sync Step QuestIDs\"."),
				Step.QuestSubID, Step.QuestID, Asset->QuestID));
		}
	}

	// Check for duplicate SubIDs
	TMap<int32, int32> SubIDCount;
	for (const FQuestStep& Step : Asset->Steps)
		SubIDCount.FindOrAdd(Step.QuestSubID)++;
	for (const auto& Pair : SubIDCount)
	{
		if (Pair.Value > 1)
			Warnings.Add(FString::Printf(TEXT("Duplicate QuestSubID %d appears %d times. Use \"Sequence SubIDs\"."),
				Pair.Key, Pair.Value));
	}

	// Check that exactly one finishing step exists (warn if none or multiple)
	int32 FinishingCount = 0;
	for (const FQuestStep& Step : Asset->Steps)
		if (Step.FinishingStep) ++FinishingCount;
	if (Asset->Steps.Num() > 0 && FinishingCount == 0)
		Warnings.Add(TEXT("No step has FinishingStep = true. The quest will never grant its reward."));
	if (FinishingCount > 1)
		Warnings.Add(FString::Printf(TEXT("%d steps have FinishingStep = true. Only the last should be finishing."), FinishingCount));

	// Check that QuestID is not 0
	if (Asset->QuestID == 0)
		Warnings.Add(TEXT("QuestID is 0. Set a unique non-zero ID before shipping."));

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

FReply FQuestAssetDetailCustomization::OnAssignNextQuestID()
{
	if (!QuestAssetPtr.IsValid())
		return FReply::Handled();

	UQuestAsset* Asset = QuestAssetPtr.Get();
	int64 MaxQuestID = FMath::Max<int64>(0, Asset->QuestID);

	FARFilter Filter;
	Filter.ClassPaths.Add(UQuestAsset::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> QuestAssetData;
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().GetAssets(Filter, QuestAssetData);

	for (const FAssetData& AssetData : QuestAssetData)
	{
		if (const UQuestAsset* ProjectQuest = Cast<UQuestAsset>(AssetData.GetAsset()))
			MaxQuestID = FMath::Max(MaxQuestID, ProjectQuest->QuestID);
	}

	if (MaxQuestID == TNumericLimits<int64>::Max())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot assign a QuestID: the project already uses INT64_MAX."));
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("AssignNextQuestIDTransaction", "Assign Next Quest ID"));
	Asset->Modify();

	const int64 NextQuestID = MaxQuestID + 1;
	Asset->QuestID = NextQuestID;
	for (FQuestStep& Step : Asset->Steps)
		Step.QuestID = NextQuestID;

	Asset->MarkPackageDirty();

	if (CachedDetailBuilder)
		CachedDetailBuilder->ForceRefreshDetails();

	return FReply::Handled();
}

FReply FQuestAssetDetailCustomization::OnSyncStepQuestIDs()
{
	if (!QuestAssetPtr.IsValid())
		return FReply::Handled();

	UQuestAsset* Asset = QuestAssetPtr.Get();
	for (FQuestStep& Step : Asset->Steps)
		Step.QuestID = Asset->QuestID;

	(void)Asset->MarkPackageDirty();

	if (CachedDetailBuilder)
		CachedDetailBuilder->ForceRefreshDetails();

	return FReply::Handled();
}

FReply FQuestAssetDetailCustomization::OnAutoSequenceSubIDs()
{
	if (!QuestAssetPtr.IsValid())
		return FReply::Handled();

	UQuestAsset* Asset = QuestAssetPtr.Get();
	for (int32 i = 0; i < Asset->Steps.Num(); ++i)
		Asset->Steps[i].QuestSubID = i;

	(void)Asset->MarkPackageDirty();

	if (CachedDetailBuilder)
		CachedDetailBuilder->ForceRefreshDetails();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

