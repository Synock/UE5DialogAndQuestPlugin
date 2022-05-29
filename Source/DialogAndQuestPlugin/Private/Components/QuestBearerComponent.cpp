// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/QuestBearerComponent.h"

#include "Interfaces/DialogGameModeInterface.h"
#include "Interfaces/QuestGiverInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UQuestBearerComponent::UQuestBearerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UQuestBearerComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::OnRep_KnownQuest()
{
	KnownQuestDispatcher.Broadcast();

	int32 LocalID = 0;
	for (const auto& QuestData : KnownQuestData)
	{
		if (!KnownQuestDataLUT.Contains(QuestData.QuestID))
			KnownQuestDataLUT.Add(QuestData.QuestID, LocalID);
		else
		{
			KnownQuestDataLUT[QuestData.QuestID] = LocalID;
		}
		++LocalID;
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::Server_TryProgressAll_Validate(AActor* Validator)
{
	if (!Validator)
		return false;

	if (!Cast<IQuestGiverInterface>(Validator))
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::Server_TryProgressAll_Implementation(AActor* Validator)
{
	const IQuestGiverInterface* ValidatorGiver = Cast<IQuestGiverInterface>(Validator);

	for (auto& Data :
	     ValidatorGiver->GetQuestGiverComponent()->GetValidatableQuestSteps())
	{
		Server_TryProgressQuest(Data.Key, Validator);
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (auto& QData = KnownQuestData[KnownQuestDataLUT[QuestMeta.QuestID]]; NextQuestStep.QuestSubID != QData.
		ProgressID)
	{
		QData.CurrentStep.Completed = true;
		QData.PreviousStep.Add(std::move(QData.CurrentStep));
		QData.ProgressID = NextQuestStep.QuestSubID;


		FQuestProgressStep NewStepProgress;
		NewStepProgress.Completed = false;
		NewStepProgress.StepDescription = NextQuestStep.StepDescription;
		NewStepProgress.StepTitle = NextQuestStep.StepTitle;
		NewStepProgress.QuestID = NextQuestStep.QuestID;
		NewStepProgress.QuestSubID = NextQuestStep.QuestSubID;
		if (NextQuestStep.FinishingStep)
		{
			NewStepProgress.Completed = NextQuestStep.FinishingStep;
			QData.Finished = true;
		}
		QData.CurrentStep = std::move(NewStepProgress);
	}

	OnRep_KnownQuest();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::AddQuest(const FQuestMetaData& QuestMeta)
{
	if (const ENetRole LocalRole = GetOwnerRole(); LocalRole == ROLE_Authority)
	{
		if (!KnownQuestDataLUT.Contains(QuestMeta.QuestID))
		{
			FQuestProgressData NewQuestData;
			NewQuestData.Repeatable = QuestMeta.Repeatable;
			NewQuestData.QuestTitle = QuestMeta.QuestTitle;
			NewQuestData.QuestID = QuestMeta.QuestID;
			NewQuestData.ProgressID = 0;
			NewQuestData.CurrentStep = {QuestMeta.Steps[0], false};
			KnownQuestData.Add(std::move(NewQuestData));

			KnownQuestDataLUT.Add(QuestMeta.QuestID, KnownQuestData.Num() - 1);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::Server_TryProgressQuest_Implementation(int64 QuestID, AActor* Validator)
{
	if (IDialogGameModeInterface* Gm = Cast<IDialogGameModeInterface>(UGameplayStatics::GetGameMode(GetWorld())))
		Gm->TryProgressQuest(QuestID, Cast<APlayerController>(GetOwner()), Validator);
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::Server_TryProgressQuest_Validate(int64 QuestID, AActor* Validator)
{
	return true;
}

//----------------------------------------------------------------------------------------------------------------------

const FQuestProgressData& UQuestBearerComponent::GetKnownQuest(int64 QuestID) const
{
	return KnownQuestData[KnownQuestDataLUT.FindChecked(QuestID)];
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsQuestKnown(int64 QuestID) const
{
	return KnownQuestDataLUT.Contains(QuestID);
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::CanDisplay(int64 QuestID, int32 StepID) const
{
	if (!IsQuestKnown(QuestID))
		return false;

	return IsAtStep(QuestID, StepID);
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::CanValidate(int64 QuestID, int32 StepID) const
{
	if (StepID == 0)
		return !IsQuestKnown(QuestID);

	if (!IsQuestKnown(QuestID))
		return false;

	if (IsPastStep(QuestID, StepID))
		return false;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsPastStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).ProgressID > StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsAtOrPastStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).ProgressID >= StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsAtStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).ProgressID == StepID;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::AuthorityAddQuest(int64 QuestID)
{
	if (const ENetRole LocalRole = GetOwnerRole(); LocalRole == ROLE_Authority)
	{
		if (!KnownQuestDataLUT.Contains(QuestID))
		{
			IDialogGameModeInterface* IDialogInterface = Cast<IDialogGameModeInterface>(
				GetWorld()->GetAuthGameMode());

			check(IDialogInterface);

			const FQuestMetaData& QuestData = IDialogInterface->GetMainQuestComponent()->GetQuestData(QuestID);
			FQuestProgressData NewQuestData;
			NewQuestData.Repeatable = QuestData.Repeatable;
			NewQuestData.QuestTitle = QuestData.QuestTitle;
			NewQuestData.QuestID = QuestData.QuestID;
			NewQuestData.ProgressID = 0;
			NewQuestData.CurrentStep = {QuestData.Steps[0], false};
			KnownQuestData.Add(std::move(NewQuestData));

			KnownQuestDataLUT.Add(QuestData.QuestID, KnownQuestData.Num() - 1);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::TryProgressQuest(int64 QuestID, AActor* Validator)
{
	Server_TryProgressQuest(QuestID, Validator);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::TryProgressAll(AActor* Validator)
{
	Server_TryProgressAll(Validator);
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UQuestBearerComponent, KnownQuestData, COND_OwnerOnly);
}
