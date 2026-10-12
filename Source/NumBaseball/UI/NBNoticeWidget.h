#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBNoticeWidget.generated.h"

class UBorder;
class UTextBlock;

/** 화면 중앙에 승리/무승부/새 게임 공지를 잠시 띄우는 위젯 */
UCLASS()
class NUMBASEBALL_API UNBNoticeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowNotice(const FString& Notice, float Duration);
	void HideNotice();

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> NoticeBorder;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NoticeText;

private:
	void BuildDefaultLayout();

	FTimerHandle HideTimerHandle;
};
