#include "Player/NBPlayerController.h"
#include "Player/NBPlayerState.h"
#include "Game/NBGameModeBase.h"
#include "UI/NBChatWidget.h"
#include "UI/NBNoticeWidget.h"
#include "Blueprint/UserWidget.h"
#include "NumBaseball.h"

ANBPlayerController::ANBPlayerController()
{
	ChatWidgetClass = UNBChatWidget::StaticClass();
	NoticeWidgetClass = UNBNoticeWidget::StaticClass();
}

void ANBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// UI 는 화면을 가진 로컬 플레이어에게만 필요하다.
	if (IsLocalController())
	{
		CreateWidgets();
	}
}

void ANBPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	RefreshGuessCountUI();
}

void ANBPlayerController::CreateWidgets()
{
	if (!ChatWidget && ChatWidgetClass)
	{
		ChatWidget = CreateWidget<UNBChatWidget>(this, ChatWidgetClass);
		ChatWidget->AddToViewport(0);
	}

	if (!NoticeWidget && NoticeWidgetClass)
	{
		NoticeWidget = CreateWidget<UNBNoticeWidget>(this, NoticeWidgetClass);
		NoticeWidget->AddToViewport(10);
	}

	if (ChatWidget)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(ChatWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		SetShowMouseCursor(true);
		ChatWidget->FocusInput();

		for (const TPair<FString, ENBMessageType>& Pending : PendingMessages)
		{
			ChatWidget->AddChatMessage(Pending.Key, GetMessageColor(Pending.Value));
		}
		PendingMessages.Reset();
	}

	RefreshGuessCountUI();
}

void ANBPlayerController::SendChatMessage(const FString& Message)
{
	// 클라이언트 단 1차 필터: 빈 문자열은 서버에 보내지 않는다.
	// (최종 검사는 반드시 서버에서 다시 한다)
	const FString Trimmed = Message.TrimStartAndEnd();
	if (Trimmed.IsEmpty())
	{
		return;
	}
	Server_SendChatMessage(Trimmed.Left(MaxMessageLength));
}

void ANBPlayerController::Server_SendChatMessage_Implementation(const FString& Message)
{
	// 조작된 클라이언트가 아주 긴 문자열을 보내도 서버 로직을 해치지 않도록 자른다.
	const FString Safe = Message.Left(MaxMessageLength);

	if (ANBGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ANBGameModeBase>())
	{
		GameMode->HandleChatMessage(this, Safe);
	}
}

void ANBPlayerController::Client_ReceiveMessage_Implementation(const FString& Message, ENBMessageType Type)
{
	AddMessageToChat(Message, Type);
}

void ANBPlayerController::AddMessageToChat(const FString& Message, ENBMessageType Type)
{
	if (ChatWidget)
	{
		ChatWidget->AddChatMessage(Message, GetMessageColor(Type));
	}
	else
	{
		PendingMessages.Emplace(Message, Type);
	}
}

void ANBPlayerController::ShowNotice(const FString& Notice, float Duration)
{
	if (NoticeWidget)
	{
		NoticeWidget->ShowNotice(Notice, Duration);
	}
}

void ANBPlayerController::RefreshGuessCountUI()
{
	if (!ChatWidget)
	{
		return;
	}

	if (const ANBPlayerState* PS = GetPlayerState<ANBPlayerState>())
	{
		ChatWidget->SetGuessCountText(
			FString::Printf(TEXT("%s  남은 기회 %s"), *PS->GetPlayerName(), *PS->GetGuessCountString()),
			PS->CanGuess());
	}
}

FLinearColor ANBPlayerController::GetMessageColor(ENBMessageType Type)
{
	switch (Type)
	{
	case ENBMessageType::Guess:  return FLinearColor(1.0f, 0.85f, 0.3f);
	case ENBMessageType::System: return FLinearColor(0.4f, 0.85f, 1.0f);
	case ENBMessageType::Error:  return FLinearColor(1.0f, 0.4f, 0.4f);
	default:                     return FLinearColor::White;
	}
}
