#pragma once

#include "CoreMinimal.h"
#include "NBTypes.generated.h"

/** 채팅창에 표시되는 메시지 종류 (색상 구분용) */
UENUM(BlueprintType)
enum class ENBMessageType : uint8
{
	Chat,    // 일반 채팅
	Guess,   // 숫자 입력 및 판정 결과
	System,  // 입장/퇴장, 게임 시작 등 공지
	Error    // 본인에게만 보이는 입력 오류 안내
};
