#include "UI/NBChatWidget.h"
#include "Player/NBPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace
{
	void SetFontSize(UTextBlock* Text, int32 Size)
	{
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = Size;
		Text->SetFont(Font);
	}
}

void UNBChatWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildDefaultLayout();
	}

	if (ChatInput)
	{
		ChatInput->SetClearKeyboardFocusOnCommit(false);
		ChatInput->OnTextCommitted.AddDynamic(this, &UNBChatWidget::OnInputCommitted);
	}
}

void UNBChatWidget::BuildDefaultLayout()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Root;

	// ── 좌측 하단: 채팅 로그 + 입력창 ──
	UBorder* ChatBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ChatBorder"));
	ChatBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.55f));
	ChatBorder->SetPadding(FMargin(12.f));

	UCanvasPanelSlot* ChatSlot = Root->AddChildToCanvas(ChatBorder);
	ChatSlot->SetAnchors(FAnchors(0.f, 1.f));
	ChatSlot->SetAlignment(FVector2D(0.f, 1.f));
	ChatSlot->SetPosition(FVector2D(24.f, -24.f));
	ChatSlot->SetSize(FVector2D(720.f, 440.f));

	UVerticalBox* ChatBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ChatBox"));
	ChatBorder->SetContent(ChatBox);

	ChatScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("ChatScrollBox"));
	UVerticalBoxSlot* ScrollSlot = ChatBox->AddChildToVerticalBox(ChatScrollBox);
	ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	ScrollSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

	UTextBlock* HelpText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HelpText"));
	HelpText->SetText(FText::FromString(TEXT("숫자 3자리(1~9, 중복X) 입력 = 추리   |   /메시지 = 일반 채팅")));
	HelpText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)));
	SetFontSize(HelpText, 12);
	UVerticalBoxSlot* HelpSlot = ChatBox->AddChildToVerticalBox(HelpText);
	HelpSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 4.f));

	ChatInput = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("ChatInput"));
	ChatInput->SetHintText(FText::FromString(TEXT("숫자 3자리를 입력하고 Enter")));
	ChatBox->AddChildToVerticalBox(ChatInput);

	// ── 우측 상단: 내 이름 / 남은 기회 ──
	UBorder* CountBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CountBorder"));
	CountBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.55f));
	CountBorder->SetPadding(FMargin(16.f, 10.f));

	UCanvasPanelSlot* CountSlot = Root->AddChildToCanvas(CountBorder);
	CountSlot->SetAnchors(FAnchors(1.f, 0.f));
	CountSlot->SetAlignment(FVector2D(1.f, 0.f));
	CountSlot->SetPosition(FVector2D(-24.f, 24.f));
	CountSlot->SetAutoSize(true);

	GuessCountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GuessCountText"));
	GuessCountText->SetText(FText::FromString(TEXT("남은 기회 [0 / 3]")));
	SetFontSize(GuessCountText, 22);
	CountBorder->SetContent(GuessCountText);
}

void UNBChatWidget::AddChatMessage(const FString& Message, const FLinearColor& Color)
{
	if (!ChatScrollBox)
	{
		return;
	}

	UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Line->SetText(FText::FromString(Message));
	Line->SetColorAndOpacity(FSlateColor(Color));
	Line->SetAutoWrapText(true);
	SetFontSize(Line, 16);

	if (UScrollBoxSlot* LineSlot = Cast<UScrollBoxSlot>(ChatScrollBox->AddChild(Line)))
	{
		LineSlot->SetPadding(FMargin(0.f, 2.f));
	}

	while (ChatScrollBox->GetChildrenCount() > MaxChatLines)
	{
		ChatScrollBox->RemoveChildAt(0);
	}
	ChatScrollBox->ScrollToEnd();
}

void UNBChatWidget::SetGuessCountText(const FString& Text, bool bCanGuess)
{
	if (GuessCountText)
	{
		GuessCountText->SetText(FText::FromString(Text));
		GuessCountText->SetColorAndOpacity(FSlateColor(bCanGuess ? FLinearColor::White : FLinearColor(1.f, 0.4f, 0.4f)));
	}
}

void UNBChatWidget::FocusInput()
{
	if (ChatInput)
	{
		ChatInput->SetKeyboardFocus();
	}
}

void UNBChatWidget::OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	if (!Text.IsEmpty())
	{
		if (ANBPlayerController* PC = GetOwningPlayer<ANBPlayerController>())
		{
			PC->SendChatMessage(Text.ToString());
		}
	}

	ChatInput->SetText(FText::GetEmpty());
	FocusInput();
}
