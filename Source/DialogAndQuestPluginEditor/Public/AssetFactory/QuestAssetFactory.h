// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "QuestAssetFactory.generated.h"

/**
 * Factory for UQuestAsset — exposes "Quest Asset" directly in the Content Browser
 * right-click → Create Advanced Asset → Gameplay category.
 */
UCLASS()
class UQuestAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UQuestAssetFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
	                                  EObjectFlags Flags, UObject* Context,
	                                  FFeedbackContext* Warn) override;

	virtual FText GetDisplayName() const override;
	virtual bool ShouldShowInNewMenu() const override { return true; }
	virtual FString GetDefaultNewAssetName() const override { return TEXT("QA_NewQuest"); }
};

