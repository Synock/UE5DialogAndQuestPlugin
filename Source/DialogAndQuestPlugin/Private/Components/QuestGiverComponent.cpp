// Copyright 2022 Maximilien (Synock) Guislain


#include "Components/QuestGiverComponent.h"


// Sets default values for this component's properties
UQuestGiverComponent::UQuestGiverComponent()
{
<<<<<<< HEAD
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
=======
	PrimaryComponentTick.bCanEverTick = false;
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

// Called when the game starts
void UQuestGiverComponent::BeginPlay()
{
	Super::BeginPlay();
<<<<<<< HEAD

	// ...
=======
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

<<<<<<< HEAD
void UQuestGiverComponent::AddValidableSteps(int64 QuestID, TArray<int32> Steps)
{
	FQuestValidableSteps localData;
	localData.QuestID = QuestID;
	localData.Steps = Steps;

	ValidableSteps.Add(QuestID,std::move(localData));
=======
void UQuestGiverComponent::AddValidatableSteps(int64 QuestID, TArray<int32> Steps)
{
	FQuestValidatableSteps LocalData;
	LocalData.QuestID = QuestID;
	LocalData.Steps = Steps;

	ValidatableSteps.Add(QuestID, std::move(LocalData));
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
}

//----------------------------------------------------------------------------------------------------------------------

bool UQuestGiverComponent::CanValidateQuestStep(int64 QuestID, int32 CurrentQuestStep)
{
<<<<<<< HEAD
	const ENetRole localRole = GetOwnerRole();
	if(localRole != ROLE_Authority)
		return false;

	if (!ValidableSteps.Contains(QuestID))
		return false;

	if (ValidableSteps.FindChecked(QuestID).Steps.Contains(CurrentQuestStep))
=======
	if (const ENetRole LocalRole = GetOwnerRole(); LocalRole != ROLE_Authority)
		return false;

	if (!ValidatableSteps.Contains(QuestID))
		return false;

	if (ValidatableSteps.FindChecked(QuestID).Steps.Contains(CurrentQuestStep))
>>>>>>> d66fcc9cdcab852f21806fd3dc4ffb935ff71e84
		return true;

	return false;
}
