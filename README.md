# 조기 화재 진압 시스템

STM32 NUCLEO-F446RE 기반 4인 미니 프로젝트. 감지팀 2명과 진화팀 2명이 센서 → 화재 판단 → 방향 제어 → 물 펌프 제어를 나누어 개발합니다.

현재는 **부품 선정 전 공통 개발 기반**입니다. PC에서 공통 C 로직을 빌드·시험할 수 있습니다. STM32에서 실행할 CubeIDE 프로젝트, HAL 드라이버, 실제 배선 및 구동은 아직 없습니다. 시간·각도 설정은 시뮬레이션용 초안이며 팀 합의와 실측 후 바꿉니다.

## 시작하기

필수: Git, CMake 3.20 이상, C99 컴파일러(GCC/Clang/MSVC), Ninja. Windows에서는 GCC와 Ninja가 PATH에 있어야 합니다.

```sh
git clone https://github.com/0731chae/stm_study_project.git
cd stm_study_project
cmake --preset host
cmake --build --preset host
ctest --preset host
```

현재 PR 브랜치에서 시작할 경우 먼저 `git switch chore/initial-development-setup`을 실행합니다. PR 병합 후에는 main을 사용합니다.

데모 실행: Windows `./build/host/fire_demo.exe`, Linux/macOS `./build/host/fire_demo`.
MSVC 사용자는 Visual Studio 개발자 터미널에서 실행합니다. ARM 컴파일러를 host 프리셋에 지정하지 마세요.

## 구조

```text
firmware/
  include/           공통 데이터 계약과 API
  src/detection.c    감지팀: 연속 샘플 검증
  src/suppression.c  진화팀: 조준·분사·쿨다운 상태
  src/fire_system.c  공통 통합: 안전 우선순위와 오류 래치
  platform/stm32/    CubeIDE 연동 안내와 하드웨어 포트 계약
sim/                 PC 데모 (가짜 센서 입력, 실제 출력 없음)
tests/               논리·경계·오류 자동 시험
docs/                요구사항 / 설계 / 테스트 / 역할 / 부품 결정 기록
.github/             자동 검사와 이슈·PR 양식
```

## 먼저 읽을 문서

1. [팀 역할·작업 방식](CONTRIBUTING.md), [개발 일정](docs/team-plan.md)
2. [시스템 요구사항 초안](docs/01-system-requirements.md)
3. [SW 설계 초안](docs/02-software-design.md), [공통 인터페이스](docs/interfaces.md)
4. [테스트 명세 및 결과](docs/03-test-specification-and-results.md)
5. [부품·배선 결정표](docs/hardware.md), [STM32 연동 순서](firmware/platform/stm32/README.md)

첨부 PDF의 항목 구조를 따라 Markdown 초안을 작성했습니다. PDF 원본은 변경하지 않았습니다. 제출용 PDF 작성 시 이 문서를 기준으로 옮기고, 미정 항목·실명·실측값을 채웁니다.

## 두 팀이 맞출 약속

- 감지팀은 `fire_sample_t`에 유효 여부, 화재 후보, 목표 각도, 실제 취득 시간을 넣습니다. 같은 샘플을 반복 전달해도 연속 감지 횟수가 늘지 않습니다.
- 진화팀은 `fire_output_t`의 펌프 요청과 목표 각도를 실제 드라이버로 변환합니다. 펌프 출력은 부팅·오류·비상정지 시 OFF가 기본입니다.
- 감지 위치를 산출할 센서 구성은 미정입니다. 단일 화재 감지 신호만으로 방향을 알 수 있다고 가정하지 않습니다. `target_valid=false`에서는 분사하지 않습니다.
- 현재 `motor_enable`은 논리 요청입니다. 서보/스텝/DC 중 어느 모터인지, 정지·위치 유지의 전기적 의미는 부품 결정 후 정의합니다.
- 통신은 STM32 → PC 상태 로그용 UART를 1차 계획으로 둡니다. 아직 구현하지 않았으므로 통신 필수 조건은 미검증입니다.

개발 상태는 PC 논리 시험과 실제 장치 시험을 구분해 기록합니다. 실제 센서, 조준 정확도, 펌프 전류·유량, 물을 이용한 진압 성능은 장치 시험 전까지 완료로 표시하지 않습니다.
