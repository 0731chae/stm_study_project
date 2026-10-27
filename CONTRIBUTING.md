# 팀 개발 규칙 (합의 전 초안)

## 4인 분담

| 역할 | 주 책임 | 주 작업 경로 | 교차 검토 |
| --- | --- | --- | --- |
| 감지 A (실명 미정) | 센서 선정·배선·취득 드라이버 | 향후 STM32 센서 포트, docs/hardware.md | 감지 B |
| 감지 B (실명 미정) | 필터·연속 감지·목표 위치, 요구사항 추적 | firmware/src/detection.c, docs/01-system-requirements.md | 진화 A |
| 진화 A (실명 미정) | 방향 모터·조준·상태 제어 | firmware/src/suppression.c, 향후 모터 포트 | 감지 B |
| 진화 B (실명 미정) | 펌프·구동 회로·통합·시험 증거 | 향후 펌프 포트, tests/, docs/03-test-specification-and-results.md | 감지 A |

공통 헤더, `fire_system.c`, `.ioc`, 핀맵 변경은 양 팀에서 각 1명 이상 확인합니다. 위 표는 GitHub 권한 제한이 아닌 작업 분담입니다. 실제 계정이 정해지면 CODEOWNERS와 리뷰 요청을 추가합니다.

## 작업 순서

1. 이슈에 FR/NFR/CON, 예상 MOD/TC, 완료 기준을 적습니다.
2. 최신 main에서 `feat/detection-...`, `feat/suppression-...`, `fix/...`, `docs/...` 브랜치를 만듭니다.
3. 작은 단위로 구현하고 `cmake --preset host`, `cmake --build --preset host`, `ctest --preset host`를 실행합니다.
4. PR에 변경 이유, 관련 요구사항, 시험 결과와 미실행 항목을 적고 1명 이상에게 검토받습니다.
5. 리뷰 후 squash merge를 기본 제안으로 합니다. 공통 API 변경은 양 팀 합의 뒤 병합합니다.

커밋 예: `feat(detection): reject stale sensor samples`, `docs: define pump power requirements`.

## 충돌 줄이기

- 같은 브랜치에서 4명이 함께 작업하지 않습니다. 공용 `develop` 브랜치는 두지 않고 main + 작업 브랜치로 시작합니다.
- `.ioc` 편집 담당은 해당 이슈의 한 명으로 지정합니다. 변경 시 생성된 코드와 핀맵을 같은 PR에 포함합니다.
- CubeMX 생성 파일은 USER CODE 구역을 사용하고, 판단 로직은 `firmware/src/`에 둡니다.
- IDE workspace, Debug/Release 산출물, 개인 경로는 커밋하지 않습니다. `.ioc`, 프로젝트 설정, 필요한 HAL·startup·링커 파일은 커밋합니다.
- 핀맵, 샘플 단위, 전원, 임계값 변경 시 요구사항·설계·시험 문서도 같이 갱신합니다. ID를 재사용하지 않습니다.
- `main` 보호와 필수 CI는 저장소 관리자 설정이 별도로 필요합니다. 이 PR은 브랜치 보호 정책을 적용하지 않습니다.

## 완료 기준

PC 논리 시험 통과, 관련 문서/추적표 갱신, 타 팀 영향 확인이 코드 병합 기준입니다. 실제 기능 완료는 보드 시험 증거까지 있어야 합니다. 센서 임계값이나 펌프 유량을 PC 테스트 결과만으로 검증 완료 처리하지 않습니다.
