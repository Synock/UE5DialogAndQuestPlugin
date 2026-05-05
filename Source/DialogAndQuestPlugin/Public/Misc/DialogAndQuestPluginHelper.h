#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DialogAndQuestPluginHelper.generated.h"

DIALOGANDQUESTPLUGIN_API DECLARE_LOG_CATEGORY_EXTERN(LogDialogAndQuest, Log, All);

UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogAndQuestPluginHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DialogAndQuest")
	static void Log(const FString& LogMessage);

	UFUNCTION(BlueprintCallable, Category = "DialogAndQuest")
	static void Warning(const FString& LogMessage);

	UFUNCTION(BlueprintCallable, Category = "DialogAndQuest")
	static void Error(const FString& LogMessage);
};
