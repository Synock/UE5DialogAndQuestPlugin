// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/DialogComponent.h"
#include "UObject/Interface.h"
#include "DialogInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UDialogInterface : public UInterface
{
	GENERATED_BODY()
};

class DIALOGANDQUESTPLUGIN_API IDialogInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetRelation() const = 0;

	virtual UDialogComponent* GetDialogComponent() const = 0;

	virtual FString GetRelationString(float Relation) const = 0;
};
