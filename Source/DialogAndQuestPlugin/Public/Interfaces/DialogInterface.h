// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Components/DialogComponent.h"
#include "UObject/Interface.h"
#include "DialogInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UDialogInterface : public UInterface
{
	GENERATED_BODY()
};

class DIALOGANDQUESTPLUGIN_API IDialogInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable)
	virtual float GetRelation(AActor* RelationWithActor) const = 0;

	UFUNCTION(BlueprintCallable)
	virtual UDialogComponent* GetDialogComponent() const = 0;

	UFUNCTION(BlueprintCallable)
	virtual FString GetRelationString(float Relation) const = 0;

	virtual bool HasDialog() const = 0;

	virtual bool CanTrade() const;

	virtual bool CanGive() const;

	virtual bool CanTrain() const;

	virtual bool CanBank() const;

	virtual FText GetCharacterNameForDialog() const = 0;
};
