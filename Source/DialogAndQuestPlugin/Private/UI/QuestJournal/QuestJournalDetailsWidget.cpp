#include "UI/QuestJournal/QuestJournalDetailsWidget.h"
#include "UI/QuestJournal/QuestJournalWindow.h"

void UQuestJournalDetailsWidget::TryAutoInit()
{
	if (!ParentJournal)
	{
		// Walk UObject outer chain: DetailsWidget → WidgetTree → QuestJournalWindow
		if (UQuestJournalWindow* Window = GetTypedOuter<UQuestJournalWindow>())
		{
			Window->InitJournal(); // sets QuestBearerComponent on us via InitDialog
			return;
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalDetailsWidget::InitDialog(UQuestJournalWindow* InputParentDialog)
{
	ParentJournal        = InputParentDialog;
	QuestBearerComponent = InputParentDialog->GetQuestComponent();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalDetailsWidget::ClearDetails()
{
	if (QuestTitleText)
		QuestTitleText->SetText(FText::GetEmpty());

	if (QuestDescriptionText)
		QuestDescriptionText->SetText(FText::GetEmpty());

	if (StepListView)
		StepListView->ClearListItems();

	OnDetailsCleared();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalDetailsWidget::DisplayQuestData(int64 QuestID)
{
	TryAutoInit();

	if (!QuestBearerComponent)
		return;

	const FQuestProgressData* Quest = QuestBearerComponent->GetKnownQuestSafe(QuestID);
	if (!Quest)
	{
		ClearDetails();
		return;
	}

	// --- Title ---
	if (QuestTitleText)
		QuestTitleText->SetText(Quest->QuestTitle);

	// --- Description (state-sensitive) ---
	if (QuestDescriptionText)
	{
		const FText Description = (Quest->State == EQuestState::Mentioned)
			? Quest->MentionedDescription
			: Quest->QuestDescription;
		QuestDescriptionText->SetText(Description);
	}

	// --- Steps ---
	if (StepListView)
	{
		StepListView->ClearListItems();

		const bool bIsRumored = (Quest->State == EQuestState::Mentioned);
		StepListView->SetVisibility(bIsRumored ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

		if (!bIsRumored)
		{
			// Previous steps — shown as completed (StepWidget applies strikethrough)
			for (const FQuestProgressStep& Step : Quest->PreviousStep)
			{
				UQuestEntryData* Entry = NewObject<UQuestEntryData>(this);
				Entry->Data   = Step;
				Entry->Parent = ParentJournal;
				StepListView->AddItem(Entry);
			}

			// Current step — only for non-terminal states
			if (!Quest->IsTerminal() && Quest->CurrentStep.QuestID != 0)
			{
				UQuestEntryData* Entry = NewObject<UQuestEntryData>(this);
				Entry->Data   = Quest->CurrentStep;
				Entry->Parent = ParentJournal;
				StepListView->AddItem(Entry);
			}
		}
	}

	OnQuestDisplayed(*Quest);
}
