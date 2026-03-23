
#include "Misc/DialogAndQuestPluginHelper.h"

DEFINE_LOG_CATEGORY(LogDialogAndQuest);

void UDialogAndQuestPluginHelper::Log(const FString& LogMessage)
{
	UE_LOG(LogDialogAndQuest, Log, TEXT("%s"), *LogMessage);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("%s"), *LogMessage));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogAndQuestPluginHelper::Warning(const FString& LogMessage)
{
	UE_LOG(LogDialogAndQuest, Warning, TEXT("%s"), *LogMessage);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("%s"), *LogMessage));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogAndQuestPluginHelper::Error(const FString& LogMessage)
{
	UE_LOG(LogDialogAndQuest, Error, TEXT("%s"), *LogMessage);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s"), *LogMessage));
}
