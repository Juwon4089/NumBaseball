#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Game/NBTypes.h"
#include "NBPlayerController.generated.h"

class UNBChatWidget;
class UNBNoticeWidget;

/**
 * 클라이언트 <-> 서버 통신 창구.
 *  - 입력 전송 : Client --(Server RPC)--> Server
 *  - 개인 안내 : Server --(Client RPC)--> 해당 Client
 * 전체 공지는 GameState 의 NetMulticast RPC 를 통해 들어온다.
 */
UCLASS()
class NUMBASEBALL_API ANBPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANBPlayerController();

	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;

	/** 위젯에서 Enter 를 눌렀을 때 호출 (로컬) */
	void SendChatMessage(const FString& Message);

	/** 채팅창에 메시지 한 줄을 추가한다 (로컬). 위젯이 없으면 생성될 때까지 보관한다. */
	void AddMessageToChat(const FString& Message, ENBMessageType Type);

	/** 화면 중앙 공지 위젯을 띄운다 (로컬) */
	void ShowNotice(const FString& Notice, float Duration);

	/** 시도 횟수 표시를 PlayerState 값으로 갱신한다 (로컬) */
	void RefreshGuessCountUI();

protected:
	UFUNCTION(Server, Reliable)
	void Server_SendChatMessage(const FString& Message);

	UFUNCTION(Client, Reliable)
	void Client_ReceiveMessage(const FString& Message, ENBMessageType Type);

	friend class ANBGameModeBase;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNBChatWidget> ChatWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNBNoticeWidget> NoticeWidgetClass;

	/** 서버가 받아들이는 메시지의 최대 길이 */
	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	int32 MaxMessageLength = 100;

private:
	void CreateWidgets();
	static FLinearColor GetMessageColor(ENBMessageType Type);

	UPROPERTY()
	TObjectPtr<UNBChatWidget> ChatWidget;

	UPROPERTY()
	TObjectPtr<UNBNoticeWidget> NoticeWidget;

	/** 위젯 생성 전에 도착한 메시지 */
	TArray<TPair<FString, ENBMessageType>> PendingMessages;
};
