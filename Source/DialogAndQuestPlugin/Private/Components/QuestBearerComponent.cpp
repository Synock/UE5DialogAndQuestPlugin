// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/QuestBearerComponent.h"

#include "GameFramework/GameModeBase.h"
#include "Interfaces/DialogDisplayInterface.h"
#include "Interfaces/DialogGameModeInterface.h"
#include "Interfaces/QuestBearerInterface.h"
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

	int32 LocalID = 0;
	for (const auto& QuestData : KnownQuestData)
	{
		if (!KnownQuestDataLUT.Contains(QuestData.QuestID))
		{
			KnownQuestDataLUT.Add(QuestData.QuestID, LocalID);
			NewQuestDispatcher.Broadcast();
		}
		else
		{
			KnownQuestDataLUT[QuestData.QuestID] = LocalID;
		}
		++LocalID;
	}

	KnownQuestDispatcher.Broadcast();
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::Authority_TryProgressQuest(int64 QuestID, AActor* Validator)
{
	if(!GetOwner()->HasAuthority())
		return false;

	if (IDialogGameModeInterface* Gm = Cast<IDialogGameModeInterface>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		const bool ProgressStatus = Gm->TryProgressQuest(QuestID, Cast<APlayerController>(GetOwner()), Validator);
		return ProgressStatus;
	}

	return false;
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

void UQuestBearerComponent::ProgressQuest(const FQuestMetaData& QuestMeta, const FQuestStep& NextQuestStep, bool SkipReward)
{
	if (GetOwnerRole() != ROLE_Authority)
		return;

	if (auto& QData = KnownQuestData[KnownQuestDataLUT[QuestMeta.QuestID]]; NextQuestStep.QuestSubID != QData.
		ProgressID || QData.Repeatable)
	{
		QData.CurrentStep.Completed = true;

		if(!SkipReward && QData.CurrentStep.RewardClass != nullptr)
			if(IQuestBearerInterface* SelfBearerInterface = Cast<IQuestBearerInterface>(GetOwner()))
				SelfBearerInterface->GrantReward(QData.CurrentStep.RewardClass);

		if(!QData.Repeatable)
		{
			QData.PreviousStep.Add(std::move(QData.CurrentStep));
			QData.ProgressID = NextQuestStep.QuestSubID;

			FQuestProgressStep NewStepProgress;
			NewStepProgress.Completed = false;
			NewStepProgress.StepDescription = NextQuestStep.StepDescription;
			NewStepProgress.StepTitle = NextQuestStep.StepTitle;
			NewStepProgress.QuestID = NextQuestStep.QuestID;
			NewStepProgress.QuestSubID = NextQuestStep.QuestSubID;
			NewStepProgress.RewardClass = NextQuestStep.RewardClass;
			NewStepProgress.NecessaryItems = NextQuestStep.NecessaryItems;
			NewStepProgress.NecessaryCoins = NextQuestStep.NecessaryCoins;
			NewStepProgress.ItemTurnInDialog = NextQuestStep.ItemTurnInDialog;
			if (NextQuestStep.FinishingStep)
			{
				NewStepProgress.Completed = NextQuestStep.FinishingStep;
				QData.Finished = true;
			}
			QData.CurrentStep = std::move(NewStepProgress);

			QuestUpdateDispatcher.Broadcast(QData.QuestID, QData.CurrentStep.QuestSubID);
		}
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
			NewQuestData.CurrentStep = FQuestProgressStep(QuestMeta.Steps[0]);
			KnownQuestData.Add(std::move(NewQuestData));

			KnownQuestDataLUT.Add(QuestMeta.QuestID, KnownQuestData.Num() - 1);

			QuestUpdateDispatcher.Broadcast(QuestMeta.QuestID, 0);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::CanValidateStepWithItems(int64 QuestID, int32 StepID, const TArray<int32>& InputItems, float InputCoins,
	TArray<int32>& OutputItems, float& OutputCoins)
{

	OutputItems = InputItems;
	OutputCoins = InputCoins;

	if(!IsAtStep(QuestID,StepID))
		return false;

	const auto& CurrentStep = GetKnownQuest(QuestID).CurrentStep;

	const bool IsExpectingSomething = !CurrentStep.NecessaryItems.IsEmpty() || CurrentStep.NecessaryCoins != 0.f;

	if(!IsExpectingSomething)
		return false;

	TArray<int32> NecessaryItems = CurrentStep.NecessaryItems;
	if(!NecessaryItems.IsEmpty())
	{
		for(const auto & ItemID: InputItems)
		{
			if(const int32 Index = NecessaryItems.Find(ItemID); Index >= 0)
			{
				NecessaryItems.RemoveAt(Index);
				OutputItems.Remove(ItemID);
				continue;
			}
		}
	}

	const bool GivenItemIsOK = NecessaryItems.IsEmpty();

	float NecessaryCoins = CurrentStep.NecessaryCoins;
	NecessaryCoins -= InputCoins;

	const bool GivenCashIsOk = NecessaryCoins <= 0.f;
	if(GivenItemIsOK && GivenCashIsOk)
		return true;

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::Server_TryProgressQuest_Implementation(int64 QuestID, AActor* Validator)
{
	Authority_TryProgressQuest(QuestID, Validator);
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

bool UQuestBearerComponent::CanDisplay(int64 QuestID, int32 StepID, EQuestStepConditionType Condition) const
{
	if (!IsQuestKnown(QuestID))
		return false;

	switch (Condition)
	{
	default:
	case EQuestStepConditionType::Equal: return IsAtStep(QuestID, StepID);
	case EQuestStepConditionType::Lesser: return IsBeforeStep(QuestID, StepID);
	case EQuestStepConditionType::LesserEqual: return IsBeforeOrAtStep(QuestID, StepID);
	case EQuestStepConditionType::Greater: return IsPastStep(QuestID, StepID);
	case EQuestStepConditionType::GreaterEqual: return IsAtOrPastStep(QuestID, StepID);
	}

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

bool UQuestBearerComponent::IsBeforeStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).QuestID > 0 && GetKnownQuest(QuestID).ProgressID < StepID;
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestBearerComponent::IsBeforeOrAtStep(int64 QuestID, int32 StepID) const
{
	return GetKnownQuest(QuestID).QuestID > 0 && GetKnownQuest(QuestID).ProgressID <= StepID;
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
	if(!IsQuestKnown(QuestID))
		return false;

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
			NewQuestData.CurrentStep = FQuestProgressStep(QuestData.Steps[0]);
			KnownQuestData.Add(std::move(NewQuestData));

			KnownQuestDataLUT.Add(QuestData.QuestID, KnownQuestData.Num() - 1);
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestBearerComponent::AuthoritySetupQuestData(int64 QuestID, int32 StepID)
{
	if(GetOwnerRole() != ROLE_Authority)
		return;

	IDialogGameModeInterface* GM =  Cast<IDialogGameModeInterface>(GetWorld()->GetAuthGameMode());

	check(GM);//You should have access to Main quest component

	UQuestMainComponent* MQC =  GM->GetMainQuestComponent();

	if(!MQC)
		return;

	AuthorityAddQuest(QuestID);
	if (KnownQuestDataLUT.Contains(QuestID))
	{
		const FQuestMetaData& Meta = MQC->GetQuestData(QuestID);
		for(int32 CurrentStepID = 0; CurrentStepID < StepID; ++CurrentStepID)
		{
			ProgressQuest(Meta, MQC->FindNextStep(Meta, CurrentStepID), true);
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
