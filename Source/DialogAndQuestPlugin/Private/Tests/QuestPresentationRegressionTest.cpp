#include "Misc/AutomationTest.h"
#include "Quest/QuestData.h"
#include "UObject/UnrealType.h"

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

#endif