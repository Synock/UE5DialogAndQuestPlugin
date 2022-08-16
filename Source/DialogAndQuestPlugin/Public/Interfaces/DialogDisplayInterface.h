// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "UI/DialogWindow.h"
#include "UObject/Interface.h"
#include "DialogDisplayInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UDialogDisplayInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class DIALOGANDQUESTPLUGIN_API IDialogDisplayInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void TriggerDialogOption(const FString& DialogTopic);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	UDialogWindow* GetDialogWindow() const;

	UFUNCTION(Client, Unreliable)
	virtual void ForceDisplayTextInDialog(const FString& TextString) = 0;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void CreateDialogWindow(UDialogComponent* DialogComponent, AActor* DialogActor);

	//this function is intended to process custom script within dialog lines, for instance to retrieve player name or whatever
	virtual FString ProcessScriptedFunction(const FString& InputString, UWorld* WorldContext) const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ToggleQuestJournal();
	//----------------------------------------------------------------------------------------------------------------------

	//UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	//static void TriggerDialogOption(const FString& DialogTopic);
};
