#include "UI/NBNoticeWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UNBNoticeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildDefaultLayout();
	}

	// 공지가 떠 있어도 채팅 입력을 막지 않도록 클릭을 통과시킨다.
	SetVisibility(ESlateVisibility::HitTestInvisible);
	HideNotice();
}

void UNBNoticeWidget::BuildDefaultLayout()
{
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	NoticeBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("NoticeBorder"));
	NoticeBorder->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.1f, 0.85f));
	NoticeBorder->SetPadding(FMargin(48.f, 28.f));

	UCanvasPanelSlot* NoticeSlot = Root->AddChildToCanvas(NoticeBorder);
	NoticeSlot->SetAnchors(FAnchors(0.5f, 0.35f));
	NoticeSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	NoticeSlot->SetAutoSize(true);

	NoticeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NoticeText"));
	NoticeText->SetJustification(ETextJustify::Center);
	NoticeText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.3f)));
	FSlateFontInfo Font = NoticeText->GetFont();
	Font.Size = 44;
	NoticeText->SetFont(Font);
	NoticeBorder->SetContent(NoticeText);
}

void UNBNoticeWidget::ShowNotice(const FString& Notice, float Duration)
{
	if (NoticeText)
	{
		NoticeText->SetText(FText::FromString(Notice));
	}
	if (NoticeBorder)
	{
		NoticeBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HideTimerHandle, this, &UNBNoticeWidget::HideNotice, FMath::Max(Duration, 0.1f), false);
	}
}

void UNBNoticeWidget::HideNotice()
{
	if (NoticeBorder)
	{
		NoticeBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}
