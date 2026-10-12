# NumBaseball

UE 5.5 C++ 멀티플레이 숫자 야구. 서버가 정한 3자리 숫자(1~9, 중복 없음)를 3번 안에 먼저 맞히면 승리합니다.

## 실행
Play 설정에서 Number of Players를 2로, Net Mode를 Listen Server로 두고 실행합니다.
- 숫자 3자리 입력: 추리
- `/메시지`: 일반 채팅

## 구조
| 클래스 | 역할 |
|---|---|
| NBGameModeBase | 정답 생성, 판정, 승리·무승부, 리셋 (서버) |
| NBPlayerState | 시도 횟수 관리 |
| NBPlayerController | Server/Client RPC |
| NBGameStateBase | Multicast로 채팅과 공지 전송 |
| NBBaseballLibrary | 입력 검사, S/B 판정 |
| NBChatWidget / NBNoticeWidget | 채팅창, 결과 공지 |
