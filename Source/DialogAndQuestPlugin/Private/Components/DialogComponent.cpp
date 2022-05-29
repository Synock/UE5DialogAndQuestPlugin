// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/DialogComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/DialogGameModeInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UDialogComponent::UDialogComponent()
{
<<<<<<< HEAD
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
=======
	PrimaryComponentTick.bCanEverTick = false;
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UDialogComponent::BeginPlay()
{
	Super::BeginPlay();
}
<<<<<<< HEAD

//----------------------------------------------------------------------------------------------------------------------

void UDialogComponent::OnRep_DialogData()
{
	for (auto& DialogData : DialogTopicData)
	{
		DialogTopic.FindOrAdd(DialogData.Id, DialogData);
		DialogTopicLUT.FindOrAdd(DialogData.Topic, DialogData.Id);
	}
=======

//----------------------------------------------------------------------------------------------------------------------

void UDialogComponent::OnRep_DialogData()
{
	for (auto& DialogData : DialogTopicData)
	{
		DialogTopic.FindOrAdd(DialogData.Id, DialogData);
		DialogTopicLUT.FindOrAdd(DialogData.Topic, DialogData.Id);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogComponent::InitDialogFromID(int64 ID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	TArray<FDialogTopicStruct> FullDialog;
	IDialogGameModeInterface* GM = Cast<IDialogGameModeInterface>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		FullDialog = GM->GetMainDialogComponent()->GetAllDialogTopicForMetaBundle(ID);

		GoodGreeting = GM->GetMainDialogComponent()->GetGoodGreeting(ID);
		BadGreeting = GM->GetMainDialogComponent()->GetBadGreeting(ID);
		GreetingLimit = GM->GetMainDialogComponent()->GetGreetingRelationLimit(ID);
	}

	for (auto& DialogData : FullDialog)
	{
		DialogTopicData.Add(DialogData);
		DialogTopic.FindOrAdd(DialogData.Id, DialogData);
		DialogTopicLUT.FindOrAdd(DialogData.Topic, DialogData.Id);
	}
}

//----------------------------------------------------------------------------------------------------------------------

const FDialogTopicStruct& UDialogComponent::GetDialogTopic(int64 ID) const
{
	return DialogTopic[ID];
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

<<<<<<< HEAD
void UDialogComponent::InitDialogFromID(int64 ID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	TArray<FDialogTopicStruct> FullDialog;
	IDialogGameModeInterface* GM = Cast<IDialogGameModeInterface>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		FullDialog = GM->GetMainDialogComponent()->GetAllDialogTopicForMetaBundle(ID);

		GoodGreeting = GM->GetMainDialogComponent()->GetGoodGreeting(ID);
		BadGreeting = GM->GetMainDialogComponent()->GetBadGreeting(ID);
		GreetingLimit = GM->GetMainDialogComponent()->GetGreetingRelationLimit(ID);
	}

	for (auto& DialogData : FullDialog)
	{
		DialogTopicData.Add(DialogData);
		DialogTopic.FindOrAdd(DialogData.Id, DialogData);
		DialogTopicLUT.FindOrAdd(DialogData.Topic, DialogData.Id);
	}
=======
int64 UDialogComponent::GetDialogTopicID(const FString& ID) const
{
	if (DialogTopicLUT.Contains(ID))
		return *DialogTopicLUT.Find(ID);

	return 0;
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

<<<<<<< HEAD
const FDialogTopicStruct& UDialogComponent::GetDialogTopic(int64 ID) const
{
	return DialogTopic[ID];
}

//----------------------------------------------------------------------------------------------------------------------

int64 UDialogComponent::GetDialogTopicID(const FString& ID) const
{
	if (DialogTopicLUT.Contains(ID))
		return *DialogTopicLUT.Find(ID);

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------

=======
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
FString UDialogComponent::ParseTextHyperlink(const FString& OriginalString, const AActor* DialogActor) const
{
	FString ActualOut;
	TArray<FString> Out;
	OriginalString.ParseIntoArray(Out,TEXT(" "), true);
<<<<<<< HEAD
	for (const auto& Word : Out)
	{
		if (DialogTopicLUT.Contains(Word) && DialogTopic.Find(*DialogTopicLUT.Find(Word))->TopicCondition.
		                                                 VerifyCondition(DialogActor))
=======
	APlayerController* Controller = Cast<APlayerController>(GetOwner());
	for (const auto& Word : Out)
	{
		if (DialogTopicLUT.Contains(Word) && DialogTopic.Find(*DialogTopicLUT.Find(Word))->TopicCondition.
		                                                 VerifyCondition(DialogActor, Controller))
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
		{
			ActualOut += L"<DialogLink id=\"" + Word + "\">" + Word + L"</> ";
		}
		else
		{
			ActualOut += Word + " ";
		}
	}

	return ActualOut;
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDialogComponent, DialogTopicData);
	DOREPLIFETIME(UDialogComponent, DialogName);
	DOREPLIFETIME(UDialogComponent, GoodGreeting);
	DOREPLIFETIME(UDialogComponent, BadGreeting);
	DOREPLIFETIME(UDialogComponent, GreetingLimit);
}
