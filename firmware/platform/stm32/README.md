# NUCLEO-F446RE 연동 준비

현재 폴더는 **포트 계약과 안내**이며 업로드 가능한 CubeIDE 프로젝트가 아닙니다. 로컬에는 CubeIDE 1.19.0과 Arm GNU 13.3.Rel1이 설치되어 있는 것을 확인했습니다. 공통 C의 Cortex-M4 컴파일은 `tools/check_arm.py`로 별도 검증할 수 있지만 링크·다운로드·장치 실행 검증은 포함하지 않습니다.

## 부품 확정 후 담당자가 할 일

1. `docs/hardware.md`에 센서/방향 산출 방식/모터/펌프/드라이버/전원/핀맵을 확정하고 양 팀에서 검토합니다.
2. STM32CubeIDE에서 NUCLEO-F446RE 기반 C 프로젝트를 `firmware/board/`에 생성합니다. 프로젝트 이름은 팀이 정하고, `.ioc` 편집 담당 1명을 둡니다. IDE와 STM32CubeF4 패키지 버전을 설계서에 기록합니다.
3. 센서 입력, UART 로그, 주기용 timer, 선택한 모터 구동 Peripheral과 출력 OFF 극성을 설정합니다. 핀·CN 위치·AF·IRQ/DMA는 실제 보드 문서와 부품 데이터시트를 대조합니다.
4. 공통 `firmware/src/*.c`를 보드 프로젝트 source 폴더로 연결하고, `firmware/include`와 `firmware/platform/stm32`를 include 경로에 추가합니다. 호스트용 `sim/`과 `tests/`의 main은 포함하지 않습니다. 연결 경로는 저장소 상대 경로를 사용합니다.
5. `fire_port.h`의 함수를 별도 `.c`에 구현합니다. HAL 핸들은 포트 내부에만 두고 공통 코드에 HAL include를 넣지 않습니다.
6. USER CODE 구역에서 출력 OFF 확보 → HAL/클록/Peripheral 초기화 → 포트 초기화 → 공통 init → 주기 호출 순서로 통합합니다. 초기화/드라이버 실패는 출력 OFF를 유지하고 오류를 래치합니다.
7. main loop 10ms, 취득 50ms를 초안으로 구현합니다. 최근 정상 샘플을 전달하되 취득 실패 시 valid=false를 즉시 반영합니다. 미취득 상태는 NULL/valid=false입니다. 타임스탬프는 취득할 때만 갱신합니다.
8. 비상정지는 센서 취득·로그보다 우선해 읽고 출력 차단 경로에 반영합니다. `fire_port_apply` 실패 시 즉시 `fire_port_force_off`를 수행하고 다음 tick에서 자동 재시도/재분사하지 않도록 포트 오류 래치를 구현합니다. 현재 core에는 IO 오류 인자가 없으므로 해당 ERR/FR/API/TC를 함께 추가해야 합니다.
9. `.ioc`, `.project`, `.cproject`, 필요한 startup/linker/HAL 소스와 상대 경로 설정을 커밋합니다. 새 clone에서 보드 빌드를 재현하고 TC-101~108로 실제 장치를 확인합니다.

`motor_enable=false`의 전기적 의미(토크 해제/위치 유지/제동)와 분사 후 노즐 자세는 모터 결정 후 확정합니다. 현재 500ms 조준 대기는 실제 도착 피드백이 아닙니다. 포트 구현 전 이 계약의 함수들을 빈 구현으로 성공 처리하지 않습니다.

## 공통 코드 ARM 컴파일 확인

Python 3.9 이상과 PATH의 `arm-none-eabi-gcc`가 있을 때:

```sh
python tools/check_arm.py --compiler arm-none-eabi-gcc
```

CubeIDE 내 컴파일러를 사용할 때 `--compiler` 뒤에 해당 `arm-none-eabi-gcc.exe` 절대 경로를 전달할 수 있습니다. 이 검사는 Cortex-M4용 object 3개 생성만 확인하며 `.elf`·`.hex` 펌웨어를 만들지 않습니다.

공식 자료: [보드 문서](https://www.st.com/en/evaluation-tools/nucleo-f446re.html), [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html).
