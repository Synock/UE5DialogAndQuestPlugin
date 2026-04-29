#include "UI/QuestJournal/QuestJournalButtonWidget.h"
#include "UI/QuestJournal/QuestJournalWindow.h"
#include "Quest/QuestData.h"

void UQuestJournalButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (QuestButton)
		QuestButton->OnClicked.AddDynamic(this, &UQuestJournalButtonWidget::SelectQuest);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalButtonWidget::SelectQuest()
{
	if (ParentJournal)
		ParentJournal->DisplayQuest(LocalData.QuestID);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalButtonWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const UQuestJournalTitleData* Data = Cast<UQuestJournalTitleData>(ListItemObject);
	if (!Data)
		return;

	LocalData   = Data->Data;
	ParentJournal = Data->Parent;

	if (QuestTitleText)
		QuestTitleText->SetText(LocalData.QuestTitle);

	OnButtonRefreshed(LocalData);
}
