#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NBBaseballLibrary.generated.h"

/** 입력 유효성 검사 결과 */
UENUM(BlueprintType)
enum class ENBGuessError : uint8
{
	None,
	EmptyInput,
	NotDigit,
	InvalidLength,
	ContainsZero,
	Duplicated
};

/** 판정 결과 (스트라이크 / 볼 개수) */
USTRUCT(BlueprintType)
struct FNBJudgeResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Baseball")
	int32 Strikes = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Baseball")
	int32 Balls = 0;

	bool IsOut() const { return Strikes == 0 && Balls == 0; }
};

/**
 * 숫자 야구 규칙만 담은 순수 함수 모음.
 * 네트워크/월드 상태에 의존하지 않으므로 GameMode(서버)에서 호출해 사용한다.
 */
UCLASS()
class NUMBASEBALL_API UNBBaseballLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr int32 DigitCount = 3;

	/** 1~9 사이의 중복되지 않는 숫자 Count개를 생성한다. */
	UFUNCTION(BlueprintCallable, Category = "Baseball")
	static TArray<int32> GenerateRandomNumbers(int32 Count = 3);

	/** 입력 문자열이 규칙(3자리, 1~9 숫자, 중복 없음)에 맞는지 검사한다. */
	UFUNCTION(BlueprintPure, Category = "Baseball")
	static ENBGuessError ValidateGuess(const FString& Input);

	/** 검사 실패 사유를 사용자 안내 문구로 변환한다. */
	UFUNCTION(BlueprintPure, Category = "Baseball")
	static FString GetGuessErrorMessage(ENBGuessError Error);

	/** 유효성 검사를 통과한 입력과 정답을 비교한다. */
	UFUNCTION(BlueprintPure, Category = "Baseball")
	static FNBJudgeResult Judge(const TArray<int32>& Answer, const FString& Guess);

	/** 판정 결과를 "1S1B", "OUT" 형식의 문자열로 만든다. */
	UFUNCTION(BlueprintPure, Category = "Baseball")
	static FString ResultToString(const FNBJudgeResult& Result);

	/** {3, 8, 6} -> "386" */
	UFUNCTION(BlueprintPure, Category = "Baseball")
	static FString NumbersToString(const TArray<int32>& Numbers);
};
