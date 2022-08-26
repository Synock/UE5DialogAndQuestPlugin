// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/DialogInterface.h"

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

bool IDialogInterface::CanBank() const
{
	return false;
}
