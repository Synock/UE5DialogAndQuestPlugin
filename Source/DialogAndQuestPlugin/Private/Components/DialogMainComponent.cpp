
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

	// Create a synthetic meta-bundle from the asset
	FDialogTopicMetaBundleStruct MetaBundle;
	MetaBundle.Id = GetTypeHash(DialogAsset->GetPathName());
	MetaBundle.GoodGreetingDialog = DialogAsset->GoodGreeting;
	MetaBundle.BadGreetingDialog = DialogAsset->BadGreeting;
	MetaBundle.MinimumRelation = DialogAsset->MinimumRelation;
	MetaBundle.MetaName = DialogAsset->AssetName;
	MetaBundle.GoodGreetingVoiceover = DialogAsset->GoodGreetingVoiceover;
	MetaBundle.BadGreetingVoiceover = DialogAsset->BadGreetingVoiceover;

	// Create a single bundle for all topics
	FDialogTopicBundleStruct Bundle;
	Bundle.Id = MetaBundle.Id + 1;
	Bundle.MetaName = DialogAsset->AssetName + TEXT("_Bundle");

	for (const auto& Topic : DialogAsset->Topics)
	{
		AddTopic(Topic);
		Bundle.TopicList.Add(Topic.Id);
	}

	AddBundle(Bundle);
	MetaBundle.TopicBundleList.Add(Bundle.Id);
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
