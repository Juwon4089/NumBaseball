#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NBPlayerState.generated.h"

/**
 * 플레이어 개별 데이터(시도 횟수)를 보관한다.
 * 값의 변경은 서버에서만 일어나고, Replication 으로 각 클라이언트에 동기화된다.
 */
UCLASS()
class NUMBASEBALL_API ANBPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ANBPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetPlayerName(const FString& S) override;
	virtual void OnRep_PlayerName() override;

	UFUNCTION(BlueprintPure, Category = "Baseball")
	int32 GetCurrentGuessCount() const { return CurrentGuessCount; }

	UFUNCTION(BlueprintPure, Category = "Baseball")
	int32 GetMaxGuessCount() const { return MaxGuessCount; }

	UFUNCTION(BlueprintPure, Category = "Baseball")
	int32 GetRemainingGuessCount() const { return FMath::Max(0, MaxGuessCount - CurrentGuessCount); }

	UFUNCTION(BlueprintPure, Category = "Baseball")
	bool CanGuess() const { return CurrentGuessCount < MaxGuessCount; }

	/** "[현재 시도 횟수 / 최대 시도 횟수]" */
	UFUNCTION(BlueprintPure, Category = "Baseball")
	FString GetGuessCountString() const;

	/** 서버 전용: 유효한 입력 1회를 소모한다. */
	void IncreaseGuessCount();

	/** 서버 전용: 새 게임을 위해 시도 횟수를 초기화한다. */
	void ResetGuessCount();

protected:
	UFUNCTION()
	void OnRep_CurrentGuessCount();

	/** 이 PlayerState 의 주인이 로컬 플레이어라면 UI 를 갱신한다. */
	void NotifyGuessCountChanged();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentGuessCount, VisibleInstanceOnly, Category = "Baseball")
	int32 CurrentGuessCount = 0;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Baseball")
	int32 MaxGuessCount = 3;
};
