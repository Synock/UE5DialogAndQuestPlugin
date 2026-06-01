
#include "Interfaces/DialogInterface.h"
#include "Components/DialogComponent.h"

bool IDialogInterface::CanTrade() const
{
	return false;
}

bool IDialogInterface::CanGive() const
{
	return true;
}

bool IDialogInterface::CanTrain() const
{
	return false;
}

bool IDialogInterface::CanTrainPlayer(const AActor* /*Player*/) const
{
	return CanTrain();
}

bool IDialogInterface::CanBank() const
{
	return false;
}

bool IDialogInterface::CanRepair() const
{
	return false;
}

float IDialogInterface::GetMaxInteractionDistance() const
{
	return 500.f;
}

FText IDialogInterface::GetContextualBadGreeting(float /*Relation*/, AActor* /*RequestingActor*/) const
{
	if (UDialogComponent* DC = GetDialogComponent())
		return DC->GetBadGreeting();
	return FText::GetEmpty();
}

