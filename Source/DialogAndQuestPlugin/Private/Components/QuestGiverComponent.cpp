// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/QuestGiverComponent.h"


// Sets default values for this component's properties
UQuestGiverComponent::UQuestGiverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UQuestGiverComponent::BeginPlay()
{
	Super::BeginPlay();
}

//----------------------------------------------------------------------------------------------------------------------

void UQuestGiverComponent::AddValidatableSteps(int64 QuestID, TArray<int32> Steps)
{
	FQuestValidatableSteps LocalData;
	LocalData.QuestID = QuestID;
	LocalData.Steps = Steps;

	ValidatableSteps.Add(QuestID, std::move(LocalData));
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestGiverComponent::CanValidateQuestStep(int64 QuestID, int32 CurrentQuestStep)
{
	if (const ENetRole LocalRole = GetOwnerRole(); LocalRole != ROLE_Authority)
		return false;

	if (!ValidatableSteps.Contains(QuestID))
		return false;

	if (ValidatableSteps.FindChecked(QuestID).Steps.Contains(CurrentQuestStep))
		return true;

	return false;
}
