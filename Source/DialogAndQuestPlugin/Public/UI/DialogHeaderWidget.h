// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "DialogWindow.h"
#include "Blueprint/UserWidget.h"
#include "DialogHeaderWidget.generated.h"

/**
 * 
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogHeaderWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly)
	UDialogWindow* ParentDialog = nullptr;

	UPROPERTY(BlueprintReadOnly)
	const UDialogComponent* DialogComponent = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void InitDialog(UDialogWindow* InputParentDialog);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetDialogName(const FString& Name);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetRelationValue(float RelationFloat);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetRelationString(const FString& Name);
};
