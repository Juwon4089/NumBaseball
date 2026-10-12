#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Library/NBBaseballLibrary.h"
#include "Game/NBTypes.h"
#include "NBGameModeBase.generated.h"

class ANBPlayerController;
class ANBPlayerState;
class ANBGameStateBase;

/**
 * 게임 규칙의 주관자. 서버에만 존재하므로 정답은 클라이언트로 절대 전송되지 않는다.
 *  - 정답 생성 / 입력 검사 / 판정 / 승리·무승부 결정 / 리셋
 */
UCLASS()
class NUMBASEBALL_API ANBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANBGameModeBase();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** PlayerController 의 Server RPC 가 호출한다. */
	void HandleChatMessage(ANBPlayerController* Sender, const FString& Message);

protected:
	/** 게임 시작 시 정답 생성 */
	void GenerateRandomNumbers();

	/** 입력값을 정답과 비교해 "1S1B", "OUT" 등의 문자열을 반환 */
	FString CheckAnswer(const FString& Input, FNBJudgeResult& OutResult) const;

	/** 승리/무승부 시 변수 초기화 후 새 게임 시작 */
	void ResetGame();

	/** 방금 입력한 플레이어의 결과로 승리/무승부 여부를 판단한다. 라운드가 끝났으면 true */
	bool JudgeGame(const ANBPlayerState* LastGuesser, const FNBJudgeResult& Result);

	/** 모든 플레이어가 기회를 소진했는가 (IgnoredPlayer 는 퇴장 중인 플레이어) */
	bool IsDraw(const APlayerState* IgnoredPlayer = nullptr) const;

	void EndRound(const FString& Notice);

	void BroadcastMessage(const FString& Message, ENBMessageType Type) const;
	void SendMessageTo(ANBPlayerController* Target, const FString& Message, ENBMessageType Type) const;

	/** 결과 공지 후 리셋까지 대기 시간(초) */
	UPROPERTY(EditDefaultsOnly, Category = "Baseball")
	float ResetDelay = 3.0f;

private:
	TArray<int32> SecretNumbers;

	/** 결과 발표 ~ 리셋 사이에는 입력을 받지 않는다. */
	bool bRoundOver = false;

	int32 PlayerCounter = 0;

	FTimerHandle ResetTimerHandle;
};
