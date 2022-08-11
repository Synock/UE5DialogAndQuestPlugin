// Copyright 2022 Maximilien (Synock) Guislain

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GenericScreenMessage.generated.h"

/**
 *
 */
UCLASS()
class DIALOGANDQUESTPLUGIN_API UGenericScreenMessage : public UUserWidget
{
public:
	UGenericScreenMessage(const FObjectInitializer& ObjectInitializer);

private:
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite)
	float LifeTime = 5.f;

	UPROPERTY(BlueprintReadWrite)
	float FadeDuration = 2.f;

	UPROPERTY(BlueprintReadOnly)
	float ElapsedTime = 0.f;

public:

	UFUNCTION(BlueprintCallable)
	void SetupDurations(float InputLifeTime, float InputFadeDuration);

	void Tick(FGeometry MyGeometry, float InDeltaTime);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintCosmetic)
	void SetText(const FText& Text);

};
