// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/DialogMainComponent.h"


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
	TArray<FName> RowNames = DataTable->GetRowNames();
	for(auto& Row : RowNames)
	{
		FDialogTopicMetaBundleStruct* Item = DataTable->FindRow<FDialogTopicMetaBundleStruct>(Row, "");
		if(Item)
			AddMetaBundle(*Item);
	}
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
	if (DialogBundle.Contains(BundleId))
	{
		for (auto& ID : DialogBundle[BundleId].TopicList)
		{
			if (DialogTopic.Find(ID))
				Out.Add(DialogTopic[ID]);
		}
	}

	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

TArray<FDialogTopicStruct> UDialogMainComponent::GetAllDialogTopicForMetaBundle(int64 BundleMetaId) const
{
	TArray<FDialogTopicStruct> Out;
	if (const auto MetaIterator = DialogMetaBundle.Find(BundleMetaId))
	{
		for (auto& ID : MetaIterator->TopicBundleList)
		{
			Out.Append(GetAllDialogTopicForBundle(ID));
		}
	}
	return Out;
}

//----------------------------------------------------------------------------------------------------------------------

FString UDialogMainComponent::GetBadGreeting(int64 BundleMetaId) const
{
	if (const auto MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->BadGreetingDialog;

	return "Error";
}

//----------------------------------------------------------------------------------------------------------------------

FString UDialogMainComponent::GetGoodGreeting(int64 BundleMetaId) const
{
	if (const auto MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->GoodGreetingDialog;

	return "Error";
}

//----------------------------------------------------------------------------------------------------------------------

float UDialogMainComponent::GetGreetingRelationLimit(int64 BundleMetaId) const
{
	if (const auto MetaIterator = DialogMetaBundle.Find(BundleMetaId))
		return MetaIterator->MinimumRelation;

	return 0.f;
}
