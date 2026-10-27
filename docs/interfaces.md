# 감지·진화 공통 인터페이스 v0.1

## 데이터 계약

정의는 `firmware/include/fire_types.h` 한 곳에서 관리합니다. 구조체 메모리를 그대로 UART로 전송하지 않습니다.

| 데이터 | 생성/소비 | 의미 |
| --- | --- | --- |
| `fire_sample_t.valid` | 센서 포트 → 감지 | 센서 취득·변환에 성공했을 때만 true |
| `alarm` | 센서 포트 → 감지 | 부품별 임계값/극성 처리를 마친 화재 후보, 실제 진압 성공 판정이 아님 |
| `target_valid`, `target_deg` | 감지/위치 포트 → 진화 | 방향 산출 성공 여부와 논리 각도 0~180도. 미확정 시 false |
| `captured_ms` | 센서 포트 → 감지 | 실제 취득 완료 시각, now_ms와 동일한 uint32_t 단조 ms 시계 |
| `fire_event_t` | 감지 → 통합/진화 | 연속 감지 여부, 목표 각도 |
| `fire_output_t` | 통합 → 출력 포트 | 상태·오류·펌프 요청·모터 요청·목표 각도 |

한 샘플의 값과 타임스탬프는 취득 후 변경하지 않습니다. 새 취득 성공 때만 captured_ms를 갱신합니다. 같은 시각의 샘플은 중복이며 감지 횟수에 추가하지 않습니다. 역순·미래·유효기간 초과 샘플은 STALE 오류입니다. 취득 간격이 유효기간을 넘으면 연속 횟수를 다시 셉니다. 감지 위치가 확인되지 않으면 90도를 임의로 넣지 않습니다.

## API

`reset_requested`는 포트 계층에서 디바운스한 수동 reset의 상승 에지에 1 tick만 true로 전달합니다. 버튼을 누르고 있는 동안 계속 true로 전달하지 않습니다. `emergency_stop`은 정지 조건이 유지되는 동안 매 tick true입니다.

| ID | 함수 / 선언 | 호출 계약 |
| --- | --- | --- |
| API-001 | `fire_config_t fire_config_default(void)` / fire_system.h | 시뮬레이션 기본값 반환. HW 인증값 아님 |
| API-002 | `void fire_system_init(fire_system_t *, const fire_config_t *)` | 부팅 시 1회, 두 포인터 필수, config 복사. 실행 중 재초기화로 오류 해제하지 않음 |
| API-003 | `fire_output_t fire_system_step(fire_system_t *, const fire_sample_t *, uint32_t now_ms, bool emergency_stop, bool reset_requested)` | 같은 system 단일 호출자, main loop 주기 호출. system 필수, sample=NULL은 센서 오류. 값으로 출력 반환 |
| API-004 | `detection_update(...)` / detection.h | 내부 API. 포인터 필수(sample은 NULL 허용), fire_error_t 반환, event 기록 |
| API-005 | `suppression_update(...)` / suppression.h | 내부 API. 통합 계층이 검증한 event만 전달. output·controller 갱신 |

모든 공통 함수는 동적 할당·HAL 의존·대기·실제 IO가 없습니다. ISR에서 호출하지 않습니다. 공통 API가 하드웨어 드라이버 성공 여부를 검증하지 않으므로, 출력 포트 실패 시 즉시 물리 출력 OFF 및 오류 래치 경로를 포트 통합 PR에서 추가해야 합니다. 포트 미완료 상태에서 자동 분사를 연결하지 않습니다.

권장 스케줄 초안: main loop 10ms, 센서 취득 50ms. ISR/DMA는 버퍼/플래그만 인계하고, main loop에서 일관된 스냅샷으로 복사합니다. UART는 제어를 지연시키지 않는 큐/비차단 전송을 사용합니다. MCU 실행 주기와 WCET는 미측정입니다.

기간값은 1~2^31-1ms, 시계 비교 간격은 2^31ms 미만이어야 합니다. `uint32_t` 뺄셈으로 한 번의 tick wrap을 지원합니다. 시간 제한은 다음 step 호출에서 평가되므로 호출이 멈췄을 때 펌프를 차단하는 watchdog/하드웨어 경로는 별도 구현·검증 대상입니다.

## UART 로그 규약 (구현 전 제안)

STM32 TX → PC 수신, 115200 baud / 8 data / no parity / 1 stop, ASCII 한 줄당 LF.

```text
FIRE,1,<ms>,<state>,<error>,<pump_0_or_1>,<motor_0_or_1>,<angle_deg>\n
```

state: IDLE/AIMING/SPRAYING/COOLDOWN/FAULT/ESTOP.
error: NONE/SENSOR/STALE/TARGET/CONFIG/ESTOP. 숫자 enum ordinal은 외부 프로토콜로 사용하지 않습니다.
주기 초안은 100ms 및 상태 변경 시, 최대 96byte/프레임. 큐가 가득 차면 로그를 버리고 드롭 횟수를 셉니다. PC는 끊긴 행·버전 불일치·필드 오류를 폐기하고 다음 LF에서 재동기화합니다. 이 링크는 관측 전용이므로 ACK/재전송/MCU 수신 명령을 두지 않습니다. 현재 PC 데모 출력은 이 UART 규약 구현물이 아닙니다.
