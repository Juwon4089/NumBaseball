#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Game/NBTypes.h"
#include "NBGameStateBase.generated.h"

/**
 * 모든 클라이언트에 존재하는 액터이므로 "전체 방송" 창구로 사용한다.
 * GameMode(서버 전용) 가 판정을 끝내면 이 클래스의 NetMulticast RPC 로 결과를 뿌린다.
 */
UCLASS()
class NUMBASEBALL_API ANBGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** 모든 플레이어의 채팅창에 메시지를 추가한다. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastMessage(const FString& Message, ENBMessageType Type);

	/** 모든 플레이어 화면에 공지 위젯(승리/무승부/새 게임)을 띄운다. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowNotice(const FString& Notice, float Duration);
};
