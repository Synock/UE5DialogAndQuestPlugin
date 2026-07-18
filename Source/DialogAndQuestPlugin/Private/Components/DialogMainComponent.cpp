
#include "Components/DialogMainComponent.h"
#include "Dialog/DialogAsset.h"


// Sets default values for this component's properties
UDialogMainComponent::UDialogMainComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UDialogMainComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogMainComponent::AddTopicFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
		return;

	TArray<FName> RowNames = DataTable->GetRowNames();
	for(auto& Row : RowNames)
	{
		FDialogTopicStruct* Item = DataTable->FindRow<FDialogTopicStruct>(Row, "");
		if(Item)
			AddTopic(*Item);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogMainComponent::AddBundleFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
		return;

	TArray<FName> RowNames = DataTable->GetRowNames();
	for(auto& Row : RowNames)
	{
		FDialogTopicBundleStruct* Item = DataTable->FindRow<FDialogTopicBundleStruct>(Row, "");
		if(Item)
			AddBundle(*Item);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogMainComponent::AddMetaBundleFromDataTable(UDataTable* DataTable)
{
	if (!DataTable)
		return;

	TArray<FName> RowNames = DataTable->GetRowNames();
	for(auto& Row : RowNames)
	{
		FDialogTopicMetaBundleStruct* Item = DataTable->FindRow<FDialogTopicMetaBundleStruct>(Row, "");
		if(Item)
			AddMetaBundle(*Item);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogMainComponent::AddFromAsset(UDialogAsset* DialogAsset)
{
	if (!DialogAsset)
		return;

	// Derive stable IDs from the asset path — same asset always maps to the same IDs.
	const int64 MetaBundleId = static_cast<int64>(GetTypeHash(DialogAsset->GetPathName()));
	const int64 OwnBundleId  = MetaBundleId + 1;

	// Idempotency guard: if this asset was already registered (e.g. a shared asset that is also
	// listed in BP_MainGameMode.DialogAssets), do nothing.
	if (DialogMetaBundle.Contains(MetaBundleId))
		return;

	// --- 1. Process shared topic assets first (recursive, depth-first) ---
	// We build a list of bundle IDs to include in our MetaBundle.
	TArray<int64> SharedBundleIds;
	for (const TSoftObjectPtr<UDialogAsset>& SharedRef : DialogAsset->SharedTopicAssets)
	{
		UDialogAsset* SharedAsset = SharedRef.LoadSynchronous();
		if (!SharedAsset)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("UDialogMainComponent::AddFromAsset — shared asset ref is null in '%s'"),
				*DialogAsset->AssetName);
			continue;
		}

		// Register the shared asset if not already done.
		AddFromAsset(SharedAsset);

		// Point our MetaBundle at the shared asset's own bundle.
		const int64 SharedBundleId = static_cast<int64>(GetTypeHash(SharedAsset->GetPathName())) + 1LL;
		SharedBundleIds.Add(SharedBundleId);
	}

	// --- 2. Build the MetaBundle for this asset ---
	FDialogTopicMetaBundleStruct MetaBundle;
	MetaBundle.Id                  = MetaBundleId;
	MetaBundle.GoodGreetingDialog  = DialogAsset->GoodGreeting;
	MetaBundle.BadGreetingDialog   = DialogAsset->BadGreeting;
	MetaBundle.MinimumRelation     = DialogAsset->MinimumRelation;
	MetaBundle.MetaName            = DialogAsset->AssetName;
	MetaBundle.GoodGreetingVoiceover = DialogAsset->GoodGreetingVoiceover;
	MetaBundle.BadGreetingVoiceover  = DialogAsset->BadGreetingVoiceover;
	MetaBundle.ConditionalGreetings  = DialogAsset->ConditionalGreetings;

	// Include shared bundles first so they appear before NPC-specific topics.
	MetaBundle.TopicBundleList.Append(SharedBundleIds);

	// --- 3. Register this asset's own topics as a separate bundle ---
	if (DialogAsset->Topics.Num() > 0)
	{
		FDialogTopicBundleStruct OwnBundle;
		OwnBundle.Id       = OwnBundleId;
		OwnBundle.MetaName = DialogAsset->AssetName + TEXT("_Bundle");

		for (const FDialogTopicStruct& Topic : DialogAsset->Topics)
		{
			AddTopic(Topic);
			OwnBundle.TopicList.Add(Topic.Id);
		}

		AddBundle(OwnBundle);
		MetaBundle.TopicBundleList.Add(OwnBundleId);
	}

	AddMetaBundle(MetaBundle);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogMainComponent::AddTopic(const FDialogTopicStruct& NewTopic)
{
	DialogTopic.Add(NewTopic.Id, NewTopic);
	DialogTopicLUT.Add(NewTopic.Topic, NewTopic.Id);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogMainComponent::AddBundle(const FDialogTopicBundleStruct& Bundle)
{
	DialogBundle.Add(Bundle.Id, Bundle);
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogMainComponent::AddMetaBundle(const FDialogTopicMetaBundleStruct& MetaBundle)
{
	DialogMetaBundle.Add(MetaBundle.Id, MetaBundle);
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FDialogTopicStruct> UDialogMainComponent::GetAllDialogTopicForBundle(int64 BundleId) const
{
	TArray<FDialogTopicStruct> Out;
	if (const FDialogTopicBundleStruct* Bundle = DialogBundle.Find(BundleId))
	{
		for (auto& ID : Bundle->TopicList)
		{
			if (const FDialogTopicStruct* Topic = DialogTopic.Find(ID))
				Out.Add(*Topic);
		}
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FDialogTopicStruct> UDialogMainComponent::GetAllDialogTopicForMetaBundle(int64 BundleMetaId) const
{
	TArray<FDialogTopicStruct> Out;
	if (const FDialogTopicMetaBundleStruct* MetaIterator = DialogMetaBundle.Find(BundleMetaId))
	{
		for (auto& ID : MetaIterator->TopicBundleList)
		{
			Out.Append(GetAllDialogTopicForBundle(ID));
		}
	}
	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

FText UDialogMainComponent::GetBadGreeting(int64 BundleMetaId) const
{
	if (const FDialogTopicMetaBundleStruct* MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->BadGreetingDialog;

	return FText::FromString(TEXT("Error"));
}

//----------------------------------------------------------------------------------------------------------------------

FText UDialogMainComponent::GetGoodGreeting(int64 BundleMetaId) const
{
	if (const FDialogTopicMetaBundleStruct* MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->GoodGreetingDialog;

	return FText::FromString(TEXT("Error"));
}

//----------------------------------------------------------------------------------------------------------------------

float UDialogMainComponent::GetGreetingRelationLimit(int64 BundleMetaId) const
{
	if (const FDialogTopicMetaBundleStruct* MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->MinimumRelation;

	return 0.f;
}

//----------------------------------------------------------------------------------------------------------------------

TSoftObjectPtr<USoundBase> UDialogMainComponent::GetGoodGreetingVoiceover(int64 BundleMetaId) const
{
	if (const FDialogTopicMetaBundleStruct* MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->GoodGreetingVoiceover;

	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

TSoftObjectPtr<USoundBase> UDialogMainComponent::GetBadGreetingVoiceover(int64 BundleMetaId) const
{
	if (const FDialogTopicMetaBundleStruct* MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->BadGreetingVoiceover;

	return nullptr;
}

TArray<FConditionalGreeting> UDialogMainComponent::GetConditionalGreetings(int64 BundleMetaId) const
{
	if (const FDialogTopicMetaBundleStruct* MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->ConditionalGreetings;

	return {};
}
