#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogConsequenceAction.generated.h"

/**
 * Typed server-side action executed by a dialog consequence after the dialog topic
 * and topic condition have already been validated by the owning PlayerController.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class DIALOGANDQUESTPLUGIN_API UDialogConsequenceAction : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Dialog|Consequence")
	void Execute(APlayerController* PlayerController, AActor* DialogActor, int64 TopicID);
	virtual void Execute_Implementation(APlayerController* PlayerController, AActor* DialogActor, int64 TopicID);
};