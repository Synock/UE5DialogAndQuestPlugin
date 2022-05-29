// Copyright 2022 Maximilien (Synock) Guislain


#include "UI/RichInlineHyperlinkDecorator.h"

#include "Framework/Text/SlateHyperlinkRun.h"
#include "Interfaces/DialogDisplayInterface.h"
#include "Widgets/Input/SRichTextHyperlink.h"

class FRichInlineHyperlinkDecorator : public FRichTextDecorator
{
protected:
	virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo,
	                                                  const FTextBlockStyle& TextStyle) const override;
	FHyperlinkStyle LinkStyle;
	FSimpleDelegate Delegate;
	mutable TMap<FString, FSimpleDelegate> RetardedTest;
	URichInlineHyperlinkDecorator* LocalDecorator;

public:
	FRichInlineHyperlinkDecorator(URichTextBlock* InOwner, class URichInlineHyperlinkDecorator* Decorator);
	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;
};

//----------------------------------------------------------------------------------------------------------------------

URichInlineHyperlinkDecorator::URichInlineHyperlinkDecorator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

//----------------------------------------------------------------------------------------------------------------------

FRichInlineHyperlinkDecorator::FRichInlineHyperlinkDecorator(URichTextBlock* InOwner,
                                                             URichInlineHyperlinkDecorator* Decorator)
	: FRichTextDecorator(InOwner)
{
	LinkStyle = Decorator->Style;
	LocalDecorator = Decorator;
}

//----------------------------------------------------------------------------------------------------------------------

bool FRichInlineHyperlinkDecorator::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const

{
	if (RunParseResult.Name == TEXT("DialogLink"))
	{
		if (RunParseResult.MetaData.Contains(TEXT("id")))
		{
			const FTextRange& IdRange = RunParseResult.MetaData[TEXT("id")];
			FString IDString = Text.Mid(IdRange.BeginIndex, IdRange.EndIndex - IdRange.BeginIndex);
			RetardedTest.FindOrAdd(IDString);
			RetardedTest.Find(IDString)->BindLambda([IDString,this]()
			{
				LocalDecorator->ClickFun(IDString);
			});
		}
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------

TSharedPtr<SWidget> FRichInlineHyperlinkDecorator::CreateDecoratorWidget(
	const FTextRunInfo& RunInfo, const FTextBlockStyle& TextStyle) const
{
	TSharedPtr<FSlateHyperlinkRun::FWidgetViewModel> Model = MakeShareable(new FSlateHyperlinkRun::FWidgetViewModel);
	TSharedPtr<SRichTextHyperlink> Link;
	if (RetardedTest.Contains(RunInfo.Content.ToString()))
	{
		Link = SNew(SRichTextHyperlink, Model.ToSharedRef())
		   .Text(RunInfo.Content)
		   .Style(&LinkStyle)
		   .OnNavigate(*RetardedTest.Find(RunInfo.Content.ToString()));
	}
	else
	{
		Link = SNew(SRichTextHyperlink, Model.ToSharedRef())
		  .Text(RunInfo.Content)
		  .Style(&LinkStyle)
		.OnNavigate(Delegate);
	}

	return Link;
}

//----------------------------------------------------------------------------------------------------------------------

TSharedPtr<ITextDecorator> URichInlineHyperlinkDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	TSharedPtr<ITextDecorator> Output = MakeShareable(new FRichInlineHyperlinkDecorator(InOwner, this));
	return Output;
}

//----------------------------------------------------------------------------------------------------------------------

void URichInlineHyperlinkDecorator::ClickFun_Implementation(const FString& ID)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Red, "click link");


	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	IDialogDisplayInterface::Execute_TriggerDialogOption(PlayerController, ID);
}
