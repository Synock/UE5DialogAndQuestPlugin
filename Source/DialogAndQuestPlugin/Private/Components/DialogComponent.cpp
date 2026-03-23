
#include "Components/DialogComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/DialogGameModeInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DialogAndQuestPluginHelper.h"
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
		auto DialogComponent = GM->GetMainDialogComponent();
		if (DialogComponent)
		{
			FullDialog = DialogComponent->GetAllDialogTopicForMetaBundle(ID);

			GoodGreeting = DialogComponent->GetGoodGreeting(ID);
			BadGreeting = DialogComponent->GetBadGreeting(ID);
			GreetingLimit = DialogComponent->GetGreetingRelationLimit(ID);
			GoodGreetingVoiceover = DialogComponent->GetGoodGreetingVoiceover(ID);
			BadGreetingVoiceover = DialogComponent->GetBadGreetingVoiceover(ID);
			bDialogInitialized = true;
		}
	}

	if (!bDialogInitialized)
		return;

	for (auto& DialogData : FullDialog)
	{
		DialogTopicData.Add(DialogData);
		DialogTopic.FindOrAdd(DialogData.Id, DialogData);
		DialogTopicLUT.FindOrAdd(DialogData.Topic, DialogData.Id);
	}
}

//----------------------------------------------------------------------------------------------------------------------

const FDialogTopicStruct* UDialogComponent::GetDialogTopicSafe(int64 ID) const
{
	if (const FDialogTopicStruct* Found = DialogTopic.Find(ID))
		return Found;

	UDialogAndQuestPluginHelper::Warning(FString::Printf(TEXT("Dialog topic ID %lld not found"), ID));
	return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------

FDialogTopicStruct UDialogComponent::GetDialogTopicByID(int64 ID, bool& bFound) const
{
	if (const FDialogTopicStruct* Found = DialogTopic.Find(ID))
	{
		bFound = true;
		return *Found;
	}

	bFound = false;
	return {};
}

//----------------------------------------------------------------------------------------------------------------------

int64 UDialogComponent::GetDialogTopicID(const FString& ID) const
{
	if (const int64* Found = DialogTopicLUT.Find(ID))
		return *Found;

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------

FString UDialogComponent::ParseTextHyperlink(const FString& OriginalString, const AActor* DialogActor,
                                             const APlayerController* Controller) const
{
	FString ActualOut;
	TArray<FString> Out;
	OriginalString.ParseIntoArray(Out, TEXT(" "), true);

	for (const auto& Word : Out)
	{
		if (Word.IsEmpty())
		{
			ActualOut += TEXT(" ");
			continue;
		}

		FString LocalWord = Word;
		TCHAR SupChar = 0;

		const TCHAR LastChar = Word[Word.Len() - 1];
		if (LastChar == '.' || LastChar == ',' || LastChar == '!' || LastChar == ':' || LastChar == '?')
		{
			SupChar = LastChar;
			LocalWord = Word.Mid(0, Word.Len() - 1);
		}

		if (!LocalWord.IsEmpty() && DialogTopicLUT.Contains(LocalWord))
		{
			const int64* TopicIDPtr = DialogTopicLUT.Find(LocalWord);
			const FDialogTopicStruct* TopicPtr = TopicIDPtr ? DialogTopic.Find(*TopicIDPtr) : nullptr;

			if (TopicPtr && TopicPtr->TopicCondition.VerifyCondition(DialogActor, Controller))
			{
				ActualOut += FString::Printf(TEXT("<DialogLink id=\"%s\">%s</>"), *LocalWord, *LocalWord);
				if (SupChar != 0)
					ActualOut += SupChar;
			}
			else
			{
				ActualOut += Word;
			}
		}
		else
		{
			ActualOut += Word;
		}

		ActualOut += TEXT(" ");
	}
	return ActualOut;
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogComponent::ConsumeTopicByID(int64 TopicID)
{
	DialogTopic.Remove(TopicID);

	// Also remove from replicated array and LUT
	for (int32 i = DialogTopicData.Num() - 1; i >= 0; --i)
	{
		if (DialogTopicData[i].Id == TopicID)
		{
			DialogTopicLUT.Remove(DialogTopicData[i].Topic);
			DialogTopicData.RemoveAt(i);
			break;
		}
	}
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
	DOREPLIFETIME(UDialogComponent, bDialogInitialized);
}
