#include "UI/QuestJournal/QuestJournalListWidget.h"
#include "UI/QuestJournal/QuestJournalWindow.h"
#include "Interfaces/QuestBearerInterface.h"

void UQuestJournalListWidget::InitDialog(UQuestJournalWindow* InputParentDialog)
{
	ParentJournal        = InputParentDialog;
	QuestBearerComponent = InputParentDialog->GetQuestComponent();

	if (QuestListView)
		QuestListView->OnItemClicked().AddUObject(this, &UQuestJournalListWidget::OnListItemClicked);

	UpdateQuestList();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalListWidget::TryAutoInit()
{
	if (!ParentJournal)
	{
		// Walk the UObject outer chain: ListWidget → WidgetTree → QuestJournalWindow
		if (UQuestJournalWindow* Window = GetTypedOuter<UQuestJournalWindow>())
		{
			// InitJournal calls InitDialog on both sub-widgets (us + DetailsWidget),
			// setting ParentJournal and QuestBearerComponent in one pass.
			// bInitialized on the window prevents duplicate delegate bindings.
			Window->InitJournal();
			return; // ParentJournal and QuestBearerComponent are now set via InitDialog
		}
	}

	// Fallback: at least resolve the data source directly if the window couldn't be found
	if (!QuestBearerComponent)
	{
		if (IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(GetOwningPlayer()))
			QuestBearerComponent = QuestBearer->GetQuestBearerComponent();
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalListWidget::UpdateQuestList()
{
	TryAutoInit();

	if (!QuestListView || !QuestBearerComponent || !ParentJournal)
		return;

	QuestListView->ClearListItems();

	for (const FQuestProgressData& QuestData : QuestBearerComponent->GetAllKnownQuest())
	{
		if (GetQuestJournalCategory(QuestData.State) != FilterCategory)
			continue;

		UQuestJournalTitleData* Entry = NewObject<UQuestJournalTitleData>(this);
		Entry->Data   = QuestData;
		Entry->Parent = ParentJournal;
		QuestListView->AddItem(Entry);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalListWidget::SetFilterCategory(EQuestJournalCategory Category)
{
	FilterCategory = Category;

	if (CategoryTitleText)
	{
		FText Title;
		switch (Category)
		{
		case EQuestJournalCategory::Active:   Title = NSLOCTEXT("QuestJournal", "Active",   "Active Quests");   break;
		case EQuestJournalCategory::Finished: Title = NSLOCTEXT("QuestJournal", "Finished", "Finished Quests"); break;
		case EQuestJournalCategory::Rumored:  Title = NSLOCTEXT("QuestJournal", "Rumored",  "Rumors");          break;
		default:                              Title = FText::GetEmpty();                                        break;
		}
		CategoryTitleText->SetText(Title);
	}

	TryAutoInit();
	UpdateQuestList();
}

//----------------------------------------------------------------------------------------------------------------------

int64 UQuestJournalListWidget::GetFirstQuestID() const
{
	if (!QuestListView || !QuestBearerComponent)
		return 0;

	for (const FQuestProgressData& QuestData : QuestBearerComponent->GetAllKnownQuest())
	{
		if (GetQuestJournalCategory(QuestData.State) == FilterCategory)
			return QuestData.QuestID;
	}
	return 0;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalListWidget::OnListItemClicked(UObject* Item)
{
	const UQuestJournalTitleData* Data = Cast<UQuestJournalTitleData>(Item);
	if (Data && ParentJournal)
		ParentJournal->DisplayQuest(Data->Data.QuestID);
}
