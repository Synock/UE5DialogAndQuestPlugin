// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/DialogComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/DialogGameModeInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UDialogComponent::UDialogComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UDialogComponent::BeginPlay()
{
	Super::BeginPlay();
}

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
}

//----------------------------------------------------------------------------------------------------------------------

int64 UDialogComponent::GetDialogTopicID(const FString& ID) const
{
	if (DialogTopicLUT.Contains(ID))
		return *DialogTopicLUT.Find(ID);

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------

FString UDialogComponent::ParseTextHyperlink(const FString& OriginalString, const AActor* DialogActor) const
{
	FString ActualOut;
	TArray<FString> Out;
	OriginalString.ParseIntoArray(Out,TEXT(" "), true);
	APlayerController* Controller = Cast<APlayerController>(GetOwner());
	for (const auto& Word : Out)
	{
		if (DialogTopicLUT.Contains(Word) && DialogTopic.Find(*DialogTopicLUT.Find(Word))->TopicCondition.
		                                                 VerifyCondition(DialogActor, Controller))
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
