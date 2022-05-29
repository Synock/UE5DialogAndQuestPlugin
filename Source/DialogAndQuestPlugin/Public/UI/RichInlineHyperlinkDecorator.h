// Copyright 2022 Maximilien (Synock) Guislain
/*
 * Original code from
 * https://forums.unrealengine.com/t/umg-richtextblock-hyperlink-href-markup/454860
 */
#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "RichInlineHyperlinkDecorator.generated.h"

UCLASS()
class DIALOGANDQUESTPLUGIN_API URichInlineHyperlinkDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()

protected :
public:
	UPROPERTY(EditAnywhere, Category=Appearance)
	FHyperlinkStyle Style;

	UFUNCTION(BlueprintNativeEvent)
	void ClickFun(const FString& ID);

	URichInlineHyperlinkDecorator(const FObjectInitializer& ObjectInitializer);

	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;
};
