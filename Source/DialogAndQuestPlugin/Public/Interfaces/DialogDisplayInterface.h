#pragma once

#include "CoreMinimal.h"
#include "Interfaces/DialogWindowInterface.h"
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

	/**
	 * Returns the game's dialog window as an IDialogWindowInterface.
	 * Implement in Blueprint by returning the UFinalDialogWindow (or any custom
	 * widget that implements IDialogWindowInterface).
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	TScriptInterface<IDialogWindowInterface> GetDialogWindow() const;

	UFUNCTION(Client, Unreliable)
	virtual void ForceDisplayTextInDialog(const FString& TextString) = 0;

	/**
	 * Display transient dialog text together with its authored voiceover metadata.
	 * The soft-object path is transported as a string because soft pointers are not
	 * reliable RPC payloads in the dialog replication path.
	 */
	UFUNCTION(Client, Unreliable)
	virtual void ForceDisplayVoicedTextInDialog(const FString& TextString, const FString& VoiceoverPath,
		FName VoiceoverEventName, float VoiceoverDuration) = 0;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void CreateDialogWindow(UDialogComponent* DialogComponent, AActor* DialogActor);

	/// Process custom script tokens within dialog lines (e.g. %p → player name).
	virtual FString ProcessScriptedFunction(const FString& InputString, UWorld* WorldContext) const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ToggleQuestJournal();
};
