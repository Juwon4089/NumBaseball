#include "Player/NBPlayerState.h"
#include "Player/NBPlayerController.h"
#include "Net/UnrealNetwork.h"

ANBPlayerState::ANBPlayerState()
{
	bReplicates = true;
}

void ANBPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANBPlayerState, CurrentGuessCount);
	DOREPLIFETIME(ANBPlayerState, MaxGuessCount);
}

void ANBPlayerState::SetPlayerName(const FString& S)
{
	Super::SetPlayerName(S);
	NotifyGuessCountChanged();
}

void ANBPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	NotifyGuessCountChanged();
}

FString ANBPlayerState::GetGuessCountString() const
{
	return FString::Printf(TEXT("[%d / %d]"), CurrentGuessCount, MaxGuessCount);
}

void ANBPlayerState::IncreaseGuessCount()
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentGuessCount = FMath::Min(CurrentGuessCount + 1, MaxGuessCount);
	// 리슨 서버의 호스트는 OnRep 이 호출되지 않으므로 직접 갱신한다.
	NotifyGuessCountChanged();
}

void ANBPlayerState::ResetGuessCount()
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentGuessCount = 0;
	NotifyGuessCountChanged();
}

void ANBPlayerState::OnRep_CurrentGuessCount()
{
	NotifyGuessCountChanged();
}

void ANBPlayerState::NotifyGuessCountChanged()
{
	// 다른 플레이어의 PlayerController 는 클라이언트에 존재하지 않으므로
	// 내 PlayerState 일 때만 캐스팅에 성공한다.
	ANBPlayerController* PC = Cast<ANBPlayerController>(GetOwningController());
	if (PC && PC->IsLocalController())
	{
		PC->RefreshGuessCountUI();
	}
}
