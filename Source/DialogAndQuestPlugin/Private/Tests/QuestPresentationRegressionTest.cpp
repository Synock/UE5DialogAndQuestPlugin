#include "Misc/AutomationTest.h"
#include "Components/DialogComponent.h"
#include "Components/QuestMainComponent.h"
#include "Quest/QuestData.h"
#include "Sound/SoundWave.h"
#include "Tests/DialogVoiceRequestTestTypes.h"
#include "UObject/UnrealType.h"

void UDialogVoiceRequestTestListener::HandleStop()
{
	++StopCount;
}

void UDialogVoiceRequestTestListener::HandleVoiceover(USoundBase* Sound, const float Duration)
{
	++VoiceCount;
	LastSound = Sound;
	LastDuration = Duration;
}

void UDialogVoiceRequestTestListener::HandleMiddleware(const FName EventName)
{
	++MiddlewareCount;
	LastEventName = EventName;
}

#if WITH_AUTOMATION_WORKER || (WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FQuestItemTurnInDialogMultilineTest,
	"DialogAndQuest.Quest.ItemTurnInDialog.Multiline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestItemTurnInDialogMultilineTest::RunTest(const FString& Parameters)
{
	const FTextProperty* ItemTurnInDialogProperty = FindFProperty<FTextProperty>(
		FQuestStep::StaticStruct(), GET_MEMBER_NAME_CHECKED(FQuestStep, ItemTurnInDialog));
	TestNotNull(TEXT("Item turn-in dialog property exists"), ItemTurnInDialogProperty);
#if WITH_METADATA
	if (ItemTurnInDialogProperty)
		TestTrue(TEXT("Item turn-in dialog uses the editor multiline control"),
			ItemTurnInDialogProperty->HasMetaData(TEXT("MultiLine")));
#endif

	FQuestStep Step;
	Step.ItemTurnInDialog = FText::FromString(TEXT("First line\nSecond line"));
	TestEqual(TEXT("Item turn-in dialog preserves authored line breaks"),
		Step.ItemTurnInDialog.ToString(), FString(TEXT("First line\nSecond line")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FQuestTurnInPresentationSelectionTest,
	"DialogAndQuest.Quest.ItemTurnInDialog.VoiceoverSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestTurnInPresentationSelectionTest::RunTest(const FString& Parameters)
{
	FQuestStep CurrentStep;
	CurrentStep.ItemTurnInDialog = FText::FromString(TEXT("Current reward"));
	CurrentStep.VoiceoverCue = TSoftObjectPtr<USoundBase>(
		FSoftObjectPath(TEXT("/Game/Test/VO_Current.VO_Current")));
	CurrentStep.VoiceoverEventName = TEXT("CurrentEvent");
	CurrentStep.VoiceoverDuration = 1.25f;

	FQuestStep NextStep;
	NextStep.ItemTurnInDialog = FText::FromString(TEXT("Destination reward"));
	NextStep.VoiceoverCue = TSoftObjectPtr<USoundBase>(
		FSoftObjectPath(TEXT("/Game/Test/VO_Destination.VO_Destination")));
	NextStep.VoiceoverEventName = TEXT("DestinationEvent");
	NextStep.VoiceoverDuration = 2.5f;

	const FQuestStep& Normal = UQuestMainComponent::Test_SelectTurnInPresentationStep(
		CurrentStep, NextStep, false, false);
	TestTrue(TEXT("Normal completion uses current-step presentation"), &Normal == &CurrentStep);
	TestTrue(TEXT("Normal completion keeps current voiceover"), Normal.VoiceoverCue == CurrentStep.VoiceoverCue);

	const FQuestStep& Branch = UQuestMainComponent::Test_SelectTurnInPresentationStep(
		CurrentStep, NextStep, true, false);
	TestTrue(TEXT("Branch completion uses destination-step presentation"), &Branch == &NextStep);
	TestEqual(TEXT("Branch completion keeps destination middleware event"),
		Branch.VoiceoverEventName, NextStep.VoiceoverEventName);

	const FQuestStep& OptionalBypass = UQuestMainComponent::Test_SelectTurnInPresentationStep(
		CurrentStep, NextStep, false, true);
	TestTrue(TEXT("Optional bypass uses destination-step presentation"), &OptionalBypass == &NextStep);
	TestEqual(TEXT("Optional bypass keeps destination duration"),
		OptionalBypass.VoiceoverDuration, NextStep.VoiceoverDuration);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDialogVoiceRequestDispatchTest,
	"DialogAndQuest.Dialog.Voiceover.SharedRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDialogVoiceRequestDispatchTest::RunTest(const FString& Parameters)
{
	UDialogComponent* Dialog = NewObject<UDialogComponent>();
	UDialogVoiceRequestTestListener* Listener = NewObject<UDialogVoiceRequestTestListener>();
	USoundWave* Sound = NewObject<USoundWave>();
	Sound->Duration = 3.f;

	Dialog->OnVoiceoverStop.AddDynamic(Listener, &UDialogVoiceRequestTestListener::HandleStop);
	Dialog->OnVoiceoverRequested.AddDynamic(Listener, &UDialogVoiceRequestTestListener::HandleVoiceover);
	Dialog->OnMiddlewareVoiceoverRequested.AddDynamic(
		Listener, &UDialogVoiceRequestTestListener::HandleMiddleware);

	const TSoftObjectPtr<USoundBase> LoadedCue(Sound);
	Dialog->RequestVoiceover(LoadedCue, TEXT("RewardEvent"), 2.75f);

	TestEqual(TEXT("A shared request stops the previous voiceover"), Listener->StopCount, 1);
	TestEqual(TEXT("A loaded cue is requested once"), Listener->VoiceCount, 1);
	TestTrue(TEXT("The requested cue is preserved"), Listener->LastSound == Sound);
	TestEqual(TEXT("The subtitle duration is preserved"), Listener->LastDuration, 2.75f);
	TestEqual(TEXT("The middleware event is requested once"), Listener->MiddlewareCount, 1);
	TestEqual(TEXT("The middleware event name is preserved"),
		Listener->LastEventName, FName(TEXT("RewardEvent")));

	Dialog->RequestVoiceover(TSoftObjectPtr<USoundBase>(), NAME_None, 0.f);
	TestEqual(TEXT("An empty request still stops the previous line"), Listener->StopCount, 2);
	TestEqual(TEXT("An empty request does not play a cue"), Listener->VoiceCount, 1);
	TestEqual(TEXT("An empty request does not fire middleware"), Listener->MiddlewareCount, 1);
	return true;
}

#endif
