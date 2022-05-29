// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/DialogComponent.h"
#include "DialogWindow.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitEvent);

/**
 * 
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UDialogWindow : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	UDialogComponent* DialogComponent = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogFooterWidget* Footer = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogHeaderWidget* Header = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogTopicWidget* TopicList = nullptr;

	UPROPERTY(BlueprintReadWrite)
	class UDialogTextWidget* TopicText = nullptr;

	UPROPERTY(BlueprintReadWrite)
	AActor* DialogActor = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float RelationValue = 0.5f;

	UPROPERTY(BlueprintReadWrite)
	FString RelationString;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PostInitRelation();

public:
	UFUNCTION(BlueprintCallable)
	const UDialogComponent* GetDialogComponent() const { return DialogComponent; }

	UFUNCTION(BlueprintCallable)
	const AActor* GetDialogActor() const { return DialogActor; }

	UFUNCTION(BlueprintCallable)
	void InitDialogWindow(UDialogComponent* InputDialogComponent, AActor* ActorDialog);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void InitDialogUI();

	UFUNCTION(BlueprintCallable)
	void DisplayDialogTopic(int64 ID);

	UFUNCTION(BlueprintCallable)
	void DisplayJournalUpdate();

	UFUNCTION(BlueprintCallable)
	void DisplayDialogTopicFromString(const FString& ID);

	UFUNCTION(BlueprintCallable)
	void CloseWindow();

	UPROPERTY(BlueprintAssignable)
	FOnExitEvent OnExit;
};

