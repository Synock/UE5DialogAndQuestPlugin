// Copyright 2022 Maximilien (Synock) Guislain


#include "Misc/DialogAndQuestPluginHelper.h"

void UDialogAndQuestPluginHelper::Log(const FString& LogMessage)
{
	UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("%s"), *LogMessage));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogAndQuestPluginHelper::Warning(const FString& LogMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT(" %s"), *LogMessage));
}

//----------------------------------------------------------------------------------------------------------------------

void UDialogAndQuestPluginHelper::Error(const FString& LogMessage)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *LogMessage);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%s"), *LogMessage));
}
