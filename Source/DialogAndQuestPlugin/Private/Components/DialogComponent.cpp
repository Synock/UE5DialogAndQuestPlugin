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
		DialogTopicLUT.FindOrAdd(DialogData.Topic.ToLower(), DialogData.Id);
	}
	MarkTopicKeysDirty();
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
		DialogTopicLUT.Add(Topic.Topic.ToLower(), Topic.Id);
	}
	MarkTopicKeysDirty();
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
		DialogTopicLUT.FindOrAdd(DialogData.Topic.ToLower(), DialogData.Id);
	}
	MarkTopicKeysDirty();
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
	// LUT keys are stored lowercase — normalize the lookup to match.
	if (const int64* Found = DialogTopicLUT.Find(ID.ToLower()))
		return *Found;

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogComponent::RebuildSortedTopicKeysIfNeeded() const
{
	if (!bTopicKeysDirty)
		return;

	SortedTopicKeys.Empty(DialogTopicLUT.Num());
	for (const auto& Pair : DialogTopicLUT)
		SortedTopicKeys.Add(Pair.Key);

	// Longest key first — ensures multi-word phrases are matched before any
	// single-word sub-match (e.g. "lost sword" is tested before "lost").
	SortedTopicKeys.Sort([](const FString& A, const FString& B)
	{
		return A.Len() > B.Len();
	});

	bTopicKeysDirty = false;
}

//----------------------------------------------------------------------------------------------------------------------

FString UDialogComponent::ParseTextHyperlink(const FString& OriginalString, const AActor* DialogActor,
                                             const APlayerController* Controller) const
{
	RebuildSortedTopicKeysIfNeeded();

	// Returns true for characters that are valid word boundaries (whitespace or punctuation).
	auto IsBoundary = [](TCHAR C) -> bool
	{
		return FChar::IsWhitespace(C)
			|| C == '.' || C == ',' || C == '!' || C == '?' || C == ':' || C == ';';
	};

	const int32 TextLen = OriginalString.Len();
	FString Result;
	Result.Reserve(TextLen * 2); // over-estimate to avoid reallocations from markup

	int32 Pos = 0;
	while (Pos < TextLen)
	{
		// Only try to match a topic keyword at word-start positions.
		const bool bAtWordStart = (Pos == 0) || IsBoundary(OriginalString[Pos - 1]);

		bool bMatched = false;
		if (bAtWordStart)
		{
			for (const FString& Key : SortedTopicKeys)
			{
				// Keys are stored lowercase; Key.Len() == the original topic length.
				const int32 KeyLen = Key.Len();
				if (KeyLen == 0 || Pos + KeyLen > TextLen)
					continue;

				// Quick first-char reject using lowercase comparison.
				if (FChar::ToLower(OriginalString[Pos]) != Key[0])
					continue;

				// Full case-insensitive substring comparison.
				if (OriginalString.Mid(Pos, KeyLen).ToLower() != Key)
					continue;

				// Verify the matched phrase ends at a word boundary.
				const int32 AfterPos = Pos + KeyLen;
				TCHAR SupChar = 0;   // trailing punctuation to re-emit after </>
				int32 AdvanceExtra = 0;

				if (AfterPos == TextLen)
				{
					// End of string — valid boundary, nothing to capture.
				}
				else if (FChar::IsWhitespace(OriginalString[AfterPos]))
				{
					// Space follows — valid boundary; space will be copied in next iteration.
				}
				else if (IsBoundary(OriginalString[AfterPos]))
				{
					// Non-space punctuation (e.g. ',' or '.') — capture it so it appears
					// after </> in the output, then skip it in the main loop.
					SupChar      = OriginalString[AfterPos];
					AdvanceExtra = 1;
				}
				else
				{
					// Mid-word — not a real boundary; reject this key.
					continue;
				}

				// Condition gate — respect quest-state / relation / item requirements.
				// LUT key is lowercase; Key is already lowercase.
				const int64* TopicIDPtr = DialogTopicLUT.Find(Key);
				const FDialogTopicStruct* TopicPtr = TopicIDPtr ? DialogTopic.Find(*TopicIDPtr) : nullptr;
				if (!TopicPtr || !TopicPtr->TopicCondition.VerifyCondition(DialogActor, Controller))
					continue; // condition failed; fall through to plain-text copy

				// id attribute = lowercase key (for LUT lookup on click).
				// Visible content = original-case text from the author's dialog string.
				const FString OriginalText = OriginalString.Mid(Pos, KeyLen);
				Result += FString::Printf(TEXT("<DialogLink id=\"%s\">%s</>"), *Key, *OriginalText);
				if (SupChar != 0)
					Result += SupChar;

				Pos += KeyLen + AdvanceExtra;
				bMatched = true;
				break;
			}
		}

		if (!bMatched)
		{
			Result += OriginalString[Pos];
			++Pos;
		}
	}

	return Result;
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
			// LUT keys are lowercase — normalize the removal key to match.
			DialogTopicLUT.Remove(DialogTopicData[i].Topic.ToLower());
			DialogTopicData.RemoveAt(i);
			break;
		}
	}
	MarkTopicKeysDirty();
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
