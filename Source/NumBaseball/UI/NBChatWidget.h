#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NBChatWidget.generated.h"

class UScrollBox;
class UEditableTextBox;
class UTextBlock;

/**
 * 채팅 로그 + 입력창 + 남은 기회 표시.
 * C++ 에서 위젯 트리를 직접 구성하므로 별도의 위젯 블루프린트 없이 동작한다.
 * (블루프린트로 상속해 같은 이름의 위젯을 배치하면 그 레이아웃을 사용한다)
 */
UCLASS()
class NUMBASEBALL_API UNBChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void AddChatMessage(const FString& Message, const FLinearColor& Color);
	void SetGuessCountText(const FString& Text, bool bCanGuess);
	void FocusInput();

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void OnInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> ChatScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> ChatInput;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GuessCountText;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	int32 MaxChatLines = 100;

private:
	void BuildDefaultLayout();
};
