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
	static const FLinearColor DefaultColor = FLinearColor::White;

	if (QuestTitleText)
	{
		QuestTitleText->SetText(FText::GetEmpty());
		QuestTitleText->SetColorAndOpacity(DefaultColor);
	}

	if (QuestDescriptionText)
	{
		QuestDescriptionText->SetText(FText::GetEmpty());
		QuestDescriptionText->SetColorAndOpacity(DefaultColor);
	}

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
	const FLinearColor BotchedColor(0.55f, 0.0f, 0.0f, 1.0f);
	const bool bBotched = (Quest->State == EQuestState::Botched);

	if (QuestTitleText)
	{
		QuestTitleText->SetText(Quest->QuestTitle);
		QuestTitleText->SetColorAndOpacity(bBotched ? BotchedColor : FLinearColor::White);
	}

	// --- Description (state-sensitive) ---
	if (QuestDescriptionText)
	{
		FText Description;
		if (Quest->State == EQuestState::Mentioned)
		{
			Description = Quest->MentionedDescription;
		}
		else if (Quest->State == EQuestState::Briefed)
		{
			// Show rumor text + full description — player spoke to giver but hasn't committed.
			// Still displayed on the Rumored journal tab.
			Description = Quest->MentionedDescription.IsEmpty()
				? Quest->QuestDescription
				: FText::Format(NSLOCTEXT("QuestJournal", "BriefedDescription", "{0}\n\n{1}"),
				                Quest->MentionedDescription, Quest->QuestDescription);
		}
		else
		{
			Description = Quest->QuestDescription;
		}
		QuestDescriptionText->SetText(Description);
		QuestDescriptionText->SetColorAndOpacity(bBotched ? BotchedColor : FLinearColor::White);
	}

	// --- Steps ---
	if (StepListView)
	{
		StepListView->ClearListItems();

		const bool bIsRumored = (Quest->State == EQuestState::Mentioned ||
		                         Quest->State == EQuestState::Briefed);
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

			// Current step — only for non-terminal states.
			// Branch dispatch nodes are routing-only stubs: they carry no author-set
			// title/description and should not appear as a list entry (doing so produces
			// an uninitialised "lorem ipsum" row that confuses the player).  Instead,
			// show the branch alternatives directly, separated by OR dividers between —
			// not before — each entry so the first alternative has no leading divider.
			if (!Quest->IsTerminal() && Quest->CurrentStep.QuestID != 0)
			{
			// Branch and Parallel dispatch nodes are routing stubs with no authored
			// title/description — skip them as direct entries and show alternatives instead.
			const bool bIsBranchDispatch =
				(Quest->CurrentStep.StepType == EQuestStepType::Branch ||
				 Quest->CurrentStep.StepType == EQuestStepType::Parallel);

				if (!bIsBranchDispatch)
				{
					UQuestEntryData* Entry = NewObject<UQuestEntryData>(this);
					Entry->Data   = Quest->CurrentStep;
					Entry->Parent = ParentJournal;
					StepListView->AddItem(Entry);
				}

				// BranchAlternatives is populated server-side when CurrentStep.StepType ==
				// Branch and is replicated via FQuestProgressData, giving the client journal
				// all available paths without querying the server-side quest registry.
				// bIsBranchSeparator lets the Blueprint step widget render a divider row.
				// The separator is inserted BETWEEN alternatives (not before the first) so
				// the list starts cleanly with the first path description.
				bool bFirstAlt = true;
				for (const FQuestProgressStep& Alt : Quest->BranchAlternatives)
				{
					if (!bFirstAlt)
					{
						UQuestEntryData* Sep = NewObject<UQuestEntryData>(this);
						Sep->bIsBranchSeparator = true;
						Sep->Parent = ParentJournal;
						StepListView->AddItem(Sep);
					}
					bFirstAlt = false;

					UQuestEntryData* AltEntry = NewObject<UQuestEntryData>(this);
					AltEntry->Data   = Alt;
					AltEntry->Parent = ParentJournal;
					StepListView->AddItem(AltEntry);
				}
			}
		}
	}

	OnQuestDisplayed(*Quest);
}
