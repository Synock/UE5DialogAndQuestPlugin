#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DialogVoiceRequestTestTypes.generated.h"

class USoundBase;

UCLASS()
class UDialogVoiceRequestTestListener : public UObject
{
	GENERATED_BODY()

public:
	int32 StopCount = 0;
	int32 VoiceCount = 0;
	int32 MiddlewareCount = 0;
	TObjectPtr<USoundBase> LastSound = nullptr;
	float LastDuration = 0.f;
	FName LastEventName = NAME_None;

	UFUNCTION()
	void HandleStop();

	UFUNCTION()
	void HandleVoiceover(USoundBase* Sound, float Duration);

	UFUNCTION()
	void HandleMiddleware(FName EventName);
};
