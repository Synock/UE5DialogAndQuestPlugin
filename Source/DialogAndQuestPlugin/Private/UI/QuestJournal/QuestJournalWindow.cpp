#include "UI/QuestJournal/QuestJournalWindow.h"
#include "Interfaces/QuestBearerInterface.h"

void UQuestJournalWindow::NativeConstruct()
{
	Super::NativeConstruct();

	if (ActiveTabButton)
		ActiveTabButton->OnClicked.AddUniqueDynamic(this, &UQuestJournalWindow::OnActiveTabClicked);

	if (FinishedTabButton)
		FinishedTabButton->OnClicked.AddUniqueDynamic(this, &UQuestJournalWindow::OnFinishedTabClicked);

	if (RumoredTabButton)
		RumoredTabButton->OnClicked.AddUniqueDynamic(this, &UQuestJournalWindow::OnRumoredTabClicked);

	// Auto-call InitJournal whenever this widget becomes visible
	OnVisibilityChanged.AddUniqueDynamic(this, &UQuestJournalWindow::HandleVisibilityChanged);

	// Eagerly initialize if the player controller is already available at construction time.
	// bInitialized guards against duplicate delegate bindings on subsequent calls.
	InitJournal();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::HandleVisibilityChanged(ESlateVisibility InVisibility)
{
	if (InVisibility == ESlateVisibility::Visible || InVisibility == ESlateVisibility::SelfHitTestInvisible)
		InitJournal();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::InitJournal()
{
	if (!bInitialized)
	{
		IQuestBearerInterface* QuestBearer = Cast<IQuestBearerInterface>(GetOwningPlayer());
		if (!QuestBearer)
			return;

		QuestComponent = QuestBearer->GetQuestBearerComponent();
		if (!QuestComponent)
			return;

		QuestComponent->KnownQuestDispatcher.AddUniqueDynamic(this, &UQuestJournalWindow::UpdateKnownQuest);

		if (ListWidget)
			ListWidget->InitDialog(this);

		if (DetailsWidget)
			DetailsWidget->InitDialog(this);

		bInitialized = true;
	}

	// Refresh every time the journal opens (first open or subsequent)
	SetActiveCategory(EQuestJournalCategory::Active);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::UpdateKnownQuest()
{
	if (!ListWidget || !QuestComponent)
		return;

	ListWidget->UpdateQuestList();

	// If the currently selected quest no longer belongs to the active category, reset selection
	if (CurrentQuestID != 0)
	{
		const FQuestProgressData* Current = QuestComponent->GetKnownQuestSafe(CurrentQuestID);
		const bool bStillValid = Current && (GetQuestJournalCategory(Current->State) == ActiveCategory);
		if (!bStillValid)
			CurrentQuestID = 0;
	}

	// Auto-select first quest if nothing is selected
	if (CurrentQuestID == 0)
		CurrentQuestID = ListWidget->GetFirstQuestID();

	if (CurrentQuestID != 0)
		DisplayQuest(CurrentQuestID);
	else if (DetailsWidget)
		DetailsWidget->ClearDetails();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::DisplayQuest(int64 ID)
{
	CurrentQuestID = ID;
	if (DetailsWidget)
		DetailsWidget->DisplayQuestData(ID);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::SetActiveCategory(EQuestJournalCategory Category)
{
	ActiveCategory = Category;
	CurrentQuestID = 0;

	if (ListWidget)
		ListWidget->SetFilterCategory(Category);

	if (ListWidget)
		CurrentQuestID = ListWidget->GetFirstQuestID();

	if (CurrentQuestID != 0)
		DisplayQuest(CurrentQuestID);
	else if (DetailsWidget)
		DetailsWidget->ClearDetails();

	OnCategoryChanged(Category);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestJournalWindow::OnActiveTabClicked()
{
	SetActiveCategory(EQuestJournalCategory::Active);
}

void UQuestJournalWindow::OnFinishedTabClicked()
{
	SetActiveCategory(EQuestJournalCategory::Finished);
}

void UQuestJournalWindow::OnRumoredTabClicked()
{
	SetActiveCategory(EQuestJournalCategory::Rumored);
}
