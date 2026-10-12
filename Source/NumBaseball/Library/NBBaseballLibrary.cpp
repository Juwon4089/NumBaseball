#include "Library/NBBaseballLibrary.h"

TArray<int32> UNBBaseballLibrary::GenerateRandomNumbers(int32 Count)
{
	Count = FMath::Clamp(Count, 1, 9);

	TArray<int32> Result;
	while (Result.Num() < Count)
	{
		const int32 Number = FMath::RandRange(1, 9);
		if (!Result.Contains(Number))
		{
			Result.Add(Number);
		}
	}
	return Result;
}

ENBGuessError UNBBaseballLibrary::ValidateGuess(const FString& Input)
{
	const FString Trimmed = Input.TrimStartAndEnd();
	if (Trimmed.IsEmpty())
	{
		return ENBGuessError::EmptyInput;
	}

	// FChar::IsDigit 는 전각 숫자 등도 허용할 수 있으므로 ASCII 범위로 직접 검사한다.
	for (const TCHAR Ch : Trimmed)
	{
		if (Ch < TEXT('0') || Ch > TEXT('9'))
		{
			return ENBGuessError::NotDigit;
		}
	}

	if (Trimmed.Len() != DigitCount)
	{
		return ENBGuessError::InvalidLength;
	}

	TArray<TCHAR> Seen;
	for (const TCHAR Ch : Trimmed)
	{
		if (Ch == TEXT('0'))
		{
			return ENBGuessError::ContainsZero;
		}
		if (Seen.Contains(Ch))
		{
			return ENBGuessError::Duplicated;
		}
		Seen.Add(Ch);
	}

	return ENBGuessError::None;
}

FString UNBBaseballLibrary::GetGuessErrorMessage(ENBGuessError Error)
{
	switch (Error)
	{
	case ENBGuessError::EmptyInput:    return TEXT("숫자를 입력해주세요.");
	case ENBGuessError::NotDigit:      return TEXT("문자가 포함되어 있습니다. 숫자만 입력해주세요.");
	case ENBGuessError::InvalidLength: return TEXT("3자리 숫자를 입력해주세요.");
	case ENBGuessError::ContainsZero:  return TEXT("1~9 사이의 숫자만 사용할 수 있습니다.");
	case ENBGuessError::Duplicated:    return TEXT("중복되지 않은 숫자를 입력해주세요.");
	default:                           return FString();
	}
}

FNBJudgeResult UNBBaseballLibrary::Judge(const TArray<int32>& Answer, const FString& Guess)
{
	FNBJudgeResult Result;

	const int32 Count = FMath::Min(Answer.Num(), Guess.Len());
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Digit = Guess[i] - TEXT('0');
		if (Answer[i] == Digit)
		{
			++Result.Strikes;
		}
		else if (Answer.Contains(Digit))
		{
			++Result.Balls;
		}
	}
	return Result;
}

FString UNBBaseballLibrary::ResultToString(const FNBJudgeResult& Result)
{
	if (Result.IsOut())
	{
		return TEXT("OUT");
	}
	return FString::Printf(TEXT("%dS%dB"), Result.Strikes, Result.Balls);
}

FString UNBBaseballLibrary::NumbersToString(const TArray<int32>& Numbers)
{
	FString Result;
	for (const int32 Number : Numbers)
	{
		Result.AppendInt(Number);
	}
	return Result;
}
