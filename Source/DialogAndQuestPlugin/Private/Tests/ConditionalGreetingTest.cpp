#include "Dialog/DialogData.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Quest/QuestAsset.h"

#if WITH_AUTOMATION_WORKER || (WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConditionalGreetingSelectionTest,
	"DialogAndQuest.ConditionalGreeting.Selection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FConditionalGreetingSelectionTest::RunTest(const FString& Parameters)
{
	APlayerController* Controller = NewObject<APlayerController>();

	FConditionalGreeting FirstGreeting;
	FirstGreeting.Id = TEXT("first");
	FirstGreeting.Text = FText::FromString(TEXT("First greeting"));

	FConditionalGreeting SecondGreeting;
	SecondGreeting.Id = TEXT("second");
	SecondGreeting.Text = FText::FromString(TEXT("Second greeting"));

	const TArray<FConditionalGreeting> Greetings = { FirstGreeting, SecondGreeting };
	const FConditionalGreeting* Selected = FindFirstMatchingConditionalGreeting(Greetings, nullptr, Controller);
	TestNotNull(TEXT("A generic greeting matches without an NPC relation"), Selected);
	if (Selected)
		TestEqual(TEXT("First matching authored rule wins"), Selected->Id, FirstGreeting.Id);

	FConditionalGreeting QuestGreeting;
	QuestGreeting.Id = TEXT("requires_quest_interface");
	QuestGreeting.Text = FText::FromString(TEXT("Quest greeting"));
	QuestGreeting.Condition.Quest = NewObject<UQuestAsset>();
	QuestGreeting.Condition.Quest->QuestID = 123;
	TestFalse(TEXT("A quest condition fails when the game controller lacks the quest interface"),
		QuestGreeting.Matches(nullptr, Controller));

	TestFalse(TEXT("A conditional greeting needs a player controller"), FirstGreeting.Matches(nullptr, nullptr));
	return true;
}

#endif
