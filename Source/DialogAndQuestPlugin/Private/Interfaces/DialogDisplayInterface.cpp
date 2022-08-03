// Copyright 2022 Maximilien (Synock) Guislain


#include "Interfaces/DialogDisplayInterface.h"


// Add default functionality here for any IDialogDisplayInterface functions that are not pure virtual.
FString IDialogDisplayInterface::ProcessScriptedFunction(const FString& InputString, UWorld* WorldContext) const
{
	return InputString;
}
