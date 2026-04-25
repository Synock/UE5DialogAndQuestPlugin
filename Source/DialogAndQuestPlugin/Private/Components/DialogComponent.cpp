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

void UDialogComponent::InitDialogFromAsset(UDialogAsset* Asset)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (!Asset)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDialogComponent::InitDialogFromAsset — null asset"));
		return;
	}

	GoodGreeting              = Asset->GoodGreeting;
	BadGreeting               = Asset->BadGreeting;
	GreetingLimit             = Asset->MinimumRelation;
	GoodGreetingVoiceover     = Asset->GoodGreetingVoiceover;
	BadGreetingVoiceover      = Asset->BadGreetingVoiceover;
	GoodGreetingVoiceoverPath = GoodGreetingVoiceover.ToSoftObjectPath().ToString();
	BadGreetingVoiceoverPath  = BadGreetingVoiceover.ToSoftObjectPath().ToString();

	// Process shared assets first so NPC-specific topics take precedence:
	// AddTopicsFromAsset skips a topic ID if already present, so the LAST caller for a given ID wins.
	// By processing shared assets into the set first, then calling AddTopicsFromAsset(Asset) last,
	// the NPC's own topics overwrite any shared-asset topics with the same ID.
	// Note: do NOT pre-add Asset to Visited here — AddTopicsFromAsset adds it when it begins
	// processing, so the circular-reference guard still works correctly.
	TSet<UDialogAsset*> Visited;

	for (const TSoftObjectPtr<UDialogAsset>& SharedRef : Asset->SharedTopicAssets)
	{
		if (UDialogAsset* Shared = SharedRef.LoadSynchronous())
			AddTopicsFromAsset(Shared, Visited);
		else
			UE_LOG(LogTemp, Warning, TEXT("UDialogComponent::InitDialogFromAsset — failed to load shared asset in '%s'"), *Asset->AssetName);
	}

	AddTopicsFromAsset(Asset, Visited);
	bDialogInitialized = true;
}

void UDialogComponent::AddTopicsFromAsset(UDialogAsset* Asset, TSet<UDialogAsset*>& Visited)
{
	if (!Asset)
		return;

	// Circular reference guard — a content error, but must not crash the server.
	if (Visited.Contains(Asset))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("UDialogComponent::AddTopicsFromAsset — circular SharedTopicAssets reference detected on '%s'. Skipping."),
			*Asset->AssetName);
		return;
	}
	Visited.Add(Asset);

	// Depth-first recursion through nested shared assets.
	for (const TSoftObjectPtr<UDialogAsset>& SharedRef : Asset->SharedTopicAssets)
	{
		if (UDialogAsset* Shared = SharedRef.LoadSynchronous())
			AddTopicsFromAsset(Shared, Visited);
	}

	for (const FDialogTopicStruct& Topic : Asset->Topics)
	{
		// Overwrite any previously-added entry with the same ID.
		// Because shared assets are processed before this asset's direct topics,
		// and because the main NPC asset is always the last AddTopicsFromAsset call
		// from InitDialogFromAsset, NPC-specific topics naturally win on ID collision.
		if (DialogTopic.Contains(Topic.Id))
		{
			// Replace in the replicated array as well.
			for (FDialogTopicStruct& Existing : DialogTopicData)
			{
				if (Existing.Id == Topic.Id)
				{
					Existing = Topic;
					break;
				}
			}
		}
		else
		{
			DialogTopicData.Add(Topic);
		}
		DialogTopic.Add(Topic.Id, Topic);
		DialogTopicLUT.Add(Topic.Topic, Topic.Id);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogComponent::InitDialogFromID(int64 ID)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	IDialogGameModeInterface* GM = Cast<IDialogGameModeInterface>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
		return;

	UDialogMainComponent* MainDialog = GM->GetMainDialogComponent();
	if (!MainDialog)
		return;

	// Guard: if the MetaBundle isn't registered yet (e.g. asset not in BP_MainGameMode.DialogAssets,
	// or InitDialog fired before StartPlay finished), do NOT mark as initialized — the NPC's retry
	// timer in InitDialog() depends on bDialogInitialized remaining false.
	if (!MainDialog->HasMetaBundle(ID))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("UDialogComponent::InitDialogFromID — MetaBundle %lld not found in DialogMainComponent. "
			     "Ensure the UDialogAsset is listed in BP_MainGameMode.DialogAssets."), ID);
		return;
	}

	GoodGreeting            = MainDialog->GetGoodGreeting(ID);
	BadGreeting             = MainDialog->GetBadGreeting(ID);
	GreetingLimit           = MainDialog->GetGreetingRelationLimit(ID);
	GoodGreetingVoiceover   = MainDialog->GetGoodGreetingVoiceover(ID);
	BadGreetingVoiceover    = MainDialog->GetBadGreetingVoiceover(ID);
	GoodGreetingVoiceoverPath = GoodGreetingVoiceover.ToSoftObjectPath().ToString();
	BadGreetingVoiceoverPath  = BadGreetingVoiceover.ToSoftObjectPath().ToString();
	bDialogInitialized      = true;

	for (const FDialogTopicStruct& DialogData : MainDialog->GetAllDialogTopicForMetaBundle(ID))
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

void UDialogComponent::OnRep_GreetingVoiceovers()
{
	if (!GoodGreetingVoiceoverPath.IsEmpty())
		GoodGreetingVoiceover = TSoftObjectPtr<USoundBase>(FSoftObjectPath(GoodGreetingVoiceoverPath));
	if (!BadGreetingVoiceoverPath.IsEmpty())
		BadGreetingVoiceover = TSoftObjectPtr<USoundBase>(FSoftObjectPath(BadGreetingVoiceoverPath));
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
	DOREPLIFETIME(UDialogComponent, GoodGreetingVoiceoverPath);
	DOREPLIFETIME(UDialogComponent, BadGreetingVoiceoverPath);
}
