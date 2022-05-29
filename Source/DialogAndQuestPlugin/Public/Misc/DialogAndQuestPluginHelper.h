// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DialogAndQuestPluginHelper.generated.h"

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogAndQuestPluginHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void Log(const FString& LogMessage);

	UFUNCTION(BlueprintCallable)
	static void Warning(const FString& LogMessage);

	UFUNCTION(BlueprintCallable)
	static void Error(const FString& LogMessage);
};
