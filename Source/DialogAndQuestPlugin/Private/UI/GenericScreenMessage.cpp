// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/GenericScreenMessage.h"

UGenericScreenMessage::UGenericScreenMessage(const FObjectInitializer& ObjectInitializer): UUserWidget(
	ObjectInitializer)
{
	SetRenderOpacity(0.f);
}

void UGenericScreenMessage::SetupDurations(float InputLifeTime, float InputFadeDuration)
{
	LifeTime = InputLifeTime;
	FadeDuration = InputFadeDuration;
}

void UGenericScreenMessage::Tick(FGeometry MyGeometry, float InDeltaTime)
{
	Super::Tick(MyGeometry, InDeltaTime);
	float Opacity = 1.f;
	//Destroy
	if (ElapsedTime > LifeTime + FadeDuration + FadeDuration)
	{
		RemoveFromParent();
		MarkAsGarbage();
		return;
	}
	//Fade out
	else if (ElapsedTime > LifeTime + FadeDuration)
	{
		float Offset = LifeTime + FadeDuration;
		Opacity = FMath::Lerp(0.f, 1.f, (ElapsedTime - Offset) / FadeDuration);
	}
	//Fade In
	else if (ElapsedTime <= FadeDuration)
	{
		Opacity = FMath::Lerp(0.f, 1.f, ElapsedTime / FadeDuration);
	}

	SetRenderOpacity(Opacity);

	ElapsedTime += InDeltaTime;
}
