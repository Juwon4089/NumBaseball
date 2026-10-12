#include "Game/NBGameStateBase.h"
#include "Player/NBPlayerController.h"
#include "Engine/World.h"

void ANBGameStateBase::Multicast_BroadcastMessage_Implementation(const FString& Message, ENBMessageType Type)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 이 머신에서 화면을 가진(로컬) 플레이어에게만 전달한다.
	// 리슨 서버라면 원격 플레이어의 PlayerController 도 순회되므로 IsLocalController 로 거른다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ANBPlayerController* PC = Cast<ANBPlayerController>(It->Get());
		if (PC && PC->IsLocalController())
		{
			PC->AddMessageToChat(Message, Type);
		}
	}
}

void ANBGameStateBase::Multicast_ShowNotice_Implementation(const FString& Notice, float Duration)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ANBPlayerController* PC = Cast<ANBPlayerController>(It->Get());
		if (PC && PC->IsLocalController())
		{
			PC->ShowNotice(Notice, Duration);
		}
	}
}
