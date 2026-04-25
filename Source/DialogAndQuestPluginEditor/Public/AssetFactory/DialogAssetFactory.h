// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DialogAssetFactory.generated.h"

/**
 * Factory for UDialogAsset — exposes "Dialog Asset" directly in the Content Browser
 * right-click → Create Advanced Asset → Gameplay category.
 */
UCLASS()
class UDialogAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UDialogAssetFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
	                                  EObjectFlags Flags, UObject* Context,
	                                  FFeedbackContext* Warn) override;

	virtual FText GetDisplayName() const override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual FString GetDefaultNewAssetName() const override { return TEXT("DA_NewNPC"); }
};

