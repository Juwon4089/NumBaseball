# ⚾ NumBaseball — 멀티플레이 숫자 야구 (UE 5.5 C++)

서버가 숨긴 1~9의 중복 없는 3자리 숫자를 **다른 플레이어보다 먼저** 맞히는 멀티플레이 게임입니다.
판정은 모두 서버(GameMode)에서 하고, 클라이언트는 입력을 보내고 결과를 화면에 보여주기만 합니다.

- 엔진: Unreal Engine 5.5
- 클래스 접두사: `NB`
- 위젯도 C++로 구성해서 **블루프린트나 에셋 없이** 클론 후 바로 빌드해 실행할 수 있습니다.

---

## 1. 실행 방법

1. `NumBaseball.uproject` 우클릭 → **Generate Visual Studio project files** → 빌드 (또는 uproject를 더블클릭해서 빌드)
2. 에디터에서 Play 옵션을 **Number of Players: 2 이상**, **Net Mode: Play As Listen Server**로 설정
   (`Config/DefaultEditorPerProjectUserSettings.ini`에 기본값 2명 / Listen Server가 들어 있습니다)
3. Play → 채팅창에 숫자 3자리를 입력하고 Enter
4. 서버가 만든 정답은 **Output Log**의 `LogNumBaseball` 카테고리에서 볼 수 있습니다.

| 입력 | 동작 |
|---|---|
| `386` | 추리 (유효하면 기회 1회 소모) |
| `/안녕하세요` | 일반 채팅 (판정 없음, 기회 소모 없음) |

---

## 2. 클래스 구조와 역할

| 클래스 | 존재 위치 | 역할 |
|---|---|---|
| `ANBGameModeBase` | **서버만** | 정답 생성, 입력 검사, 판정, 승리·무승부 판단, 리셋 |
| `ANBGameStateBase` | 서버 + 모든 클라 | `NetMulticast` RPC로 모든 플레이어에게 채팅과 공지를 전송 |
| `ANBPlayerState` | 서버 + 모든 클라 | 플레이어별 시도 횟수 (`Replicated`), Getter 함수 |
| `ANBPlayerController` | 서버 + 소유 클라 | `Server` RPC로 입력 전송, `Client` RPC로 개인 안내 수신, 위젯 관리 |
| `UNBBaseballLibrary` | (순수 함수) | 난수 생성, 유효성 검사, S/B 판정, 결과 문자열 변환 (C++ 라이브러리) |
| `UNBChatWidget` | 로컬 클라 | 채팅 로그, 입력창, `이름 남은 기회 [n / 3]` 표시 |
| `UNBNoticeWidget` | 로컬 클라 | 승리/무승부/새 게임 공지 |

> 정답 `SecretNumbers`는 GameMode에만 있으므로 **클라이언트로 복제되지 않습니다.**
> 그래서 클라이언트 메모리를 들여다봐도 정답을 알아낼 수 없습니다.

---

## 3. 데이터 흐름 (RPC 배치)

```
[Client A]                         [Server]                                  [All Clients]
 ChatWidget
   │ Enter "123"
   ▼
 NBPlayerController
   │ Server_SendChatMessage("123")  ──▶  NBPlayerController (서버 측 인스턴스)
   │      (Server, Reliable)               │ 길이 제한(100자)
                                           ▼
                                     NBGameModeBase::HandleChatMessage
                                           │
                    ┌──────────────────────┼─────────────────────────────┐
                    │ "/..." 일반 채팅      │ 입력 오류 / 기회 없음         │ 유효한 입력
                    ▼                      ▼                             ▼
          Multicast_BroadcastMessage   Client_ReceiveMessage       PS->IncreaseGuessCount()
                    │                 (본인에게만, 기회 유지)        │   └ CurrentGuessCount 복제 ──▶ OnRep → UI 갱신
                    │                                              CheckAnswer() → "0S1B"
                    │                                              Multicast_BroadcastMessage("A: 123 -> 0S1B [1/3]")
                    │                                              JudgeGame()
                    │                                               ├ 3S → EndRound("A 승리!")
                    │                                               ├ 모두 소진 → EndRound("무승부!")
                    │                                               └ 그 외 → 계속
                    │                                                     │
                    │                                   EndRound: Multicast_ShowNotice ──▶ NoticeWidget 표시
                    │                                             3초 타이머 → ResetGame()
                    │                                                     ├ 모든 PS 시도 횟수 0
                    │                                                     ├ GenerateRandomNumbers()
                    │                                                     └ Multicast_ShowNotice("새 게임 시작!")
                    ▼
            모든 클라이언트 ChatWidget에 출력  ◀──────────────────────────────────
```

| RPC | 지정자 | 방향 | 용도 |
|---|---|---|---|
| `ANBPlayerController::Server_SendChatMessage` | `Server, Reliable` | 클라 → 서버 | 채팅/추리 입력 |
| `ANBPlayerController::Client_ReceiveMessage` | `Client, Reliable` | 서버 → 해당 클라 | 입력 오류, 기회 소진 같은 개인 안내 |
| `ANBGameStateBase::Multicast_BroadcastMessage` | `NetMulticast, Reliable` | 서버 → 전체 | 채팅, 판정 결과, 입장/퇴장 |
| `ANBGameStateBase::Multicast_ShowNotice` | `NetMulticast, Reliable` | 서버 → 전체 | 승리/무승부/새 게임 공지 위젯 |

Multicast를 PlayerController가 아니라 **GameState**에 둔 이유는, PlayerController가 소유한 클라이언트에만 존재해서 다른 플레이어에게는 전달되지 않기 때문입니다. GameState는 모든 클라이언트에 있습니다.

---

## 4. 필수 기능 체크리스트

- [x] 기본 멀티플레이 채팅 (`/메시지`)
- [x] 정답 생성: `GenerateRandomNumbers()`, 1~9 중복 없는 3자리, `TArray::Contains` 사용, `UE_LOG`로 출력
- [x] 입력 검사: 3자리인지, 문자가 섞였는지, 0이 들어갔는지, 중복이 있는지. 실패하면 "다시 입력하세요" 안내를 보내고 기회는 차감하지 않음
- [x] 판정: `CheckAnswer()`가 `"1S1B"`, `"OUT"` 형식으로 반환
- [x] PlayerState로 시도 횟수 관리 (기본 3회, 복제, Getter, `[현재 / 최대]` 표시)
- [x] 승리 판정: 3S를 먼저 맞힌 플레이어를 즉시 Winner로 공지
- [x] 무승부 판정: 모든 플레이어가 기회를 소진하면 무승부 공지
- [x] 리셋: 공지 3초 후 시도 횟수를 초기화하고 새 정답 생성 (`ResetGame()`)

## 5. 예외 처리

| 상황 | 처리 |
|---|---|
| `abc`, `1a2` | "문자가 포함되어 있습니다. 숫자만 입력해주세요." (기회 유지) |
| `12`, `1234` | "3자리 숫자를 입력해주세요." (기회 유지) |
| `112` | "중복되지 않은 숫자를 입력해주세요." (기회 유지) |
| `103` | "1~9 사이의 숫자만 사용할 수 있습니다." (기회 유지) |
| `" 386 "` | 앞뒤 공백을 제거하고 정상 처리 |
| 기회 0회에서 입력 | "기회를 모두 사용했습니다 [3 / 3]. 다른 플레이어를 기다려주세요." |
| 결과 발표 중 입력 | "결과 발표 중입니다." (판정하지 않음) |
| 조작된 클라이언트가 매우 긴 문자열 전송 | 서버에서 100자로 자름. 검사는 항상 서버에서 다시 수행 |
| 남은 플레이어가 모두 소진된 상태에서 누군가 퇴장 | `Logout`에서 무승부 재판정 |
| 위젯이 생성되기 전에 도착한 메시지 | 대기열에 보관했다가 위젯 생성 후 출력 |

## 6. 추가 구현

- 메시지 종류별 색상 (채팅 / 판정 / 시스템 / 오류)
- 입장/퇴장 알림, 입장 시 규칙 안내
- 블루프린트 없이 C++로 위젯 트리를 구성 (`BindWidgetOptional`이라 BP로 상속해 디자인을 바꿀 수도 있음)
- 판정 규칙을 `UBlueprintFunctionLibrary`로 분리해서 네트워크 코드와 게임 규칙을 나눔
