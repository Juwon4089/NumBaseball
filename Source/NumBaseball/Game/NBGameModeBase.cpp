#include "Game/NBGameModeBase.h"
#include "Game/NBGameStateBase.h"
#include "Player/NBPlayerController.h"
#include "Player/NBPlayerState.h"
#include "TimerManager.h"
#include "NumBaseball.h"

ANBGameModeBase::ANBGameModeBase()
{
	GameStateClass = ANBGameStateBase::StaticClass();
	PlayerStateClass = ANBPlayerState::StaticClass();
	PlayerControllerClass = ANBPlayerController::StaticClass();
}

void ANBGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	GenerateRandomNumbers();
}

void ANBGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	++PlayerCounter;
	const FString PlayerName = FString::Printf(TEXT("Player%d"), PlayerCounter);
	if (ANBPlayerState* PS = NewPlayer->GetPlayerState<ANBPlayerState>())
	{
		PS->SetPlayerName(PlayerName);
	}

	BroadcastMessage(FString::Printf(TEXT("%s 님이 입장했습니다."), *PlayerName), ENBMessageType::System);

	ANBPlayerController* PC = Cast<ANBPlayerController>(NewPlayer);
	SendMessageTo(PC, TEXT("[숫자 야구] 서버가 숨긴 1~9 중복 없는 3자리 숫자를 맞혀보세요."), ENBMessageType::System);
	SendMessageTo(PC, TEXT("숫자 3자리 입력 = 추리 (기회 3번) / '/'로 시작 = 일반 채팅  예) /안녕하세요"), ENBMessageType::System);
}

void ANBGameModeBase::Logout(AController* Exiting)
{
	const APlayerState* ExitingPS = Exiting ? Exiting->PlayerState.Get() : nullptr;
	if (ExitingPS)
	{
		BroadcastMessage(FString::Printf(TEXT("%s 님이 퇴장했습니다."), *ExitingPS->GetPlayerName()), ENBMessageType::System);
	}

	// 남은 사람들이 모두 기회를 소진한 상태라면 기다릴 사람이 없으므로 무승부 처리
	if (!bRoundOver && IsDraw(ExitingPS))
	{
		EndRound(FString::Printf(TEXT("무승부!\n정답은 %s"), *UNBBaseballLibrary::NumbersToString(SecretNumbers)));
	}

	Super::Logout(Exiting);
}

void ANBGameModeBase::HandleChatMessage(ANBPlayerController* Sender, const FString& Message)
{
	if (!Sender)
	{
		return;
	}

	ANBPlayerState* PS = Sender->GetPlayerState<ANBPlayerState>();
	if (!PS)
	{
		return;
	}

	const FString Input = Message.TrimStartAndEnd();
	if (Input.IsEmpty())
	{
		return;
	}

	const FString PlayerName = PS->GetPlayerName();

	// 1) 일반 채팅: '/' 로 시작하면 판정 없이 모두에게 전달
	if (Input.StartsWith(TEXT("/")))
	{
		const FString ChatText = Input.RightChop(1).TrimStart();
		if (!ChatText.IsEmpty())
		{
			BroadcastMessage(FString::Printf(TEXT("%s: %s"), *PlayerName, *ChatText), ENBMessageType::Chat);
		}
		return;
	}

	// 2) 결과 발표 ~ 리셋 대기 중
	if (bRoundOver)
	{
		SendMessageTo(Sender, TEXT("결과 발표 중입니다. 잠시 후 새 게임이 시작됩니다."), ENBMessageType::Error);
		return;
	}

	// 3) 기회 소진 (기회 0회인데 억지로 입력한 경우)
	if (!PS->CanGuess())
	{
		SendMessageTo(Sender,
			FString::Printf(TEXT("기회를 모두 사용했습니다 %s. 다른 플레이어를 기다려주세요."), *PS->GetGuessCountString()),
			ENBMessageType::Error);
		return;
	}

	// 4) 입력 검사: 실패하면 기회를 차감하지 않는다.
	const ENBGuessError Error = UNBBaseballLibrary::ValidateGuess(Input);
	if (Error != ENBGuessError::None)
	{
		SendMessageTo(Sender,
			FString::Printf(TEXT("[입력 오류] \"%s\" -> %s 다시 입력하세요. (기회 유지 %s)"),
				*Input, *UNBBaseballLibrary::GetGuessErrorMessage(Error), *PS->GetGuessCountString()),
			ENBMessageType::Error);
		return;
	}

	// 5) 유효한 입력: 기회 소모 후 판정
	PS->IncreaseGuessCount();

	FNBJudgeResult Result;
	const FString ResultString = CheckAnswer(Input, Result);

	UE_LOG(LogNumBaseball, Log, TEXT("[Server] %s 입력 %s -> %s %s"),
		*PlayerName, *Input, *ResultString, *PS->GetGuessCountString());

	BroadcastMessage(
		FString::Printf(TEXT("%s: %s -> %s  %s"), *PlayerName, *Input, *ResultString, *PS->GetGuessCountString()),
		ENBMessageType::Guess);

	JudgeGame(PS, Result);
}

void ANBGameModeBase::GenerateRandomNumbers()
{
	SecretNumbers = UNBBaseballLibrary::GenerateRandomNumbers(UNBBaseballLibrary::DigitCount);

	UE_LOG(LogNumBaseball, Warning, TEXT("[Server] 새 정답 생성: %s"), *UNBBaseballLibrary::NumbersToString(SecretNumbers));
}

FString ANBGameModeBase::CheckAnswer(const FString& Input, FNBJudgeResult& OutResult) const
{
	OutResult = UNBBaseballLibrary::Judge(SecretNumbers, Input);
	return UNBBaseballLibrary::ResultToString(OutResult);
}

bool ANBGameModeBase::JudgeGame(const ANBPlayerState* LastGuesser, const FNBJudgeResult& Result)
{
	const FString Answer = UNBBaseballLibrary::NumbersToString(SecretNumbers);

	// 승리: 기회 안에 3S 를 먼저 달성한 플레이어
	if (LastGuesser && Result.Strikes == UNBBaseballLibrary::DigitCount)
	{
		UE_LOG(LogNumBaseball, Warning, TEXT("[Server] Winner: %s"), *LastGuesser->GetPlayerName());
		EndRound(FString::Printf(TEXT("%s 승리!\n정답은 %s"), *LastGuesser->GetPlayerName(), *Answer));
		return true;
	}

	// 무승부: 모든 플레이어가 기회를 소진
	if (IsDraw())
	{
		UE_LOG(LogNumBaseball, Warning, TEXT("[Server] Draw"));
		EndRound(FString::Printf(TEXT("무승부!\n정답은 %s"), *Answer));
		return true;
	}

	return false;
}

bool ANBGameModeBase::IsDraw(const APlayerState* IgnoredPlayer) const
{
	if (!GameState)
	{
		return false;
	}

	int32 ActivePlayers = 0;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const ANBPlayerState* PS = Cast<ANBPlayerState>(PlayerState);
		if (!PS || PS == IgnoredPlayer)
		{
			continue;
		}

		++ActivePlayers;
		if (PS->CanGuess())
		{
			return false;
		}
	}
	return ActivePlayers > 0;
}

void ANBGameModeBase::EndRound(const FString& Notice)
{
	bRoundOver = true;

	if (ANBGameStateBase* GS = GetGameState<ANBGameStateBase>())
	{
		GS->Multicast_ShowNotice(Notice, ResetDelay);
	}
	BroadcastMessage(Notice.Replace(TEXT("\n"), TEXT(" - ")), ENBMessageType::System);
	BroadcastMessage(FString::Printf(TEXT("%.0f초 후 새 게임이 시작됩니다."), ResetDelay), ENBMessageType::System);

	GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &ANBGameModeBase::ResetGame, ResetDelay, false);
}

void ANBGameModeBase::ResetGame()
{
	if (GameState)
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			if (ANBPlayerState* PS = Cast<ANBPlayerState>(PlayerState))
			{
				PS->ResetGuessCount();
			}
		}
	}

	GenerateRandomNumbers();
	bRoundOver = false;

	if (ANBGameStateBase* GS = GetGameState<ANBGameStateBase>())
	{
		GS->Multicast_ShowNotice(TEXT("새 게임 시작!"), 1.5f);
	}
	BroadcastMessage(TEXT("===== 새 게임이 시작되었습니다! 새로운 정답을 맞혀보세요. ====="), ENBMessageType::System);
}

void ANBGameModeBase::BroadcastMessage(const FString& Message, ENBMessageType Type) const
{
	if (ANBGameStateBase* GS = GetGameState<ANBGameStateBase>())
	{
		GS->Multicast_BroadcastMessage(Message, Type);
	}
}

void ANBGameModeBase::SendMessageTo(ANBPlayerController* Target, const FString& Message, ENBMessageType Type) const
{
	if (Target)
	{
		Target->Client_ReceiveMessage(Message, Type);
	}
}
